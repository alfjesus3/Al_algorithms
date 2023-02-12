#include <iostream>
#include <math.h>
#include <random>

#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>

#include "limbo/kernel/matern_five_halves.hpp"
#include "limbo/mean/data.hpp"
#include "limbo/model/gp.hpp"

#include "matplotlibcpp.h"


// Since `matplotlib-cpp` only works with `std::array` 
template<typename T, size_t N>
std::vector<T> convert_array_to_vector(std::array<T,N> arr) {
    std::vector<T> new_vector(arr.begin(), arr.end());
    return new_vector; 
}


///////////
//// For limbo lib
//////////

namespace limbo {
struct Params {
    // struct acqui_gpucb : public defaults::acqui_gpucb {};
    struct kernel : public defaults::kernel {
        BO_PARAM(double, noise, 0.001);
    };
    struct kernel_maternfivehalves {
        BO_PARAM(double, sigma_sq, 1);
        BO_PARAM(double, l, 0.2);
    };
};
} // namespace limbo


template <size_t N_TR_SAMPLES, size_t N_TE_SAMPLES>
struct regression_data {
    std::array<float, N_TR_SAMPLES> x_train;
    std::array<float, N_TR_SAMPLES> y_train_gt;
    std::array<float, N_TR_SAMPLES> y_train_noisy;
    std::array<float, N_TE_SAMPLES> x_test;
    std::array<float, N_TE_SAMPLES> y_test_gt;
    std::array<float, N_TE_SAMPLES> y_test_noisy;
    int num_indu_pts;
};

template <size_t N_TR_SAMPLES, size_t N_TE_SAMPLES>
regression_data<N_TR_SAMPLES, N_TE_SAMPLES> get_regression_data(
    int num_train_samples,
    int num_test_samples,
    int num_inducing_points,
    float x_train_min,
    float delta_x,
    float x_test_min
    ) {
    regression_data<N_TR_SAMPLES, N_TE_SAMPLES> reg_data;

    auto func1 = [] (float in_x) {
        return 1. * sin(in_x * 3 * M_PI) +
            .3 * cos(in_x * 9 * M_PI) +
            .5 * sin(in_x * 7 * M_PI);
    };

    // Preparing noise normal distribution
    std::random_device rd {};
    std::mt19937 gen{rd()};
    std::normal_distribution<> noise{0, 1};
    float alpha = .10;  // e.g. 10% noise in the sensor readings

    float x_val_tr = x_train_min;
    for (int i=0; i< num_train_samples; i++) {
        reg_data.x_train[i] = x_val_tr;
        reg_data.y_train_gt[i] = func1(x_val_tr);
        reg_data.y_train_noisy[i] = func1(x_val_tr) + alpha * noise(gen);

        x_val_tr += delta_x;
    }
    reg_data.num_indu_pts = num_inducing_points;

    float x_val_te = x_test_min;
    for (int j=0; j< num_test_samples; j++) {
        reg_data.x_test[j] = x_val_te;
        reg_data.y_test_gt[j] = func1(x_val_te);
        reg_data.y_test_noisy[j] = func1(x_val_te) + alpha * noise(gen);

        x_val_te += delta_x;
    }

    return reg_data;
}



int main() {
    // Adapting from deps/limbo/src/examples/obs_multi.cpp
    using Kernel_t = limbo::kernel::MaternFiveHalves<limbo::Params>;
    using Mean_t = limbo::mean::Data<limbo::Params>;
    using GP_t = limbo::model::GP<limbo::Params, Kernel_t, Mean_t>;

    namespace plt = matplotlibcpp;
    
    std::cout << "Preparing the reg data\n";
    const size_t num_train = 100;
    const size_t num_test = 15;
    const int num_inducing = 5;
    float min_x_tr = 2.;
    float delta_x = .01;
    float min_x_test = 3.0 - delta_x;

    auto reg_data = get_regression_data<num_train, num_test>(
        num_train, num_test, num_inducing,
        min_x_tr, delta_x, min_x_test);
    
    for (int i=0; i < reg_data.y_train_gt.size(); i++) {
        auto y_train_gt = reg_data.y_train_gt[i];
        auto y_train_sample = reg_data.y_train_noisy[i];
        std::cout << "sample i: " << y_train_gt << ", " << y_train_sample << "\n";
    }

    // Plotting the toy dataset
    plt::figure_size(1200, 780);
    plt::named_plot("train_gt", convert_array_to_vector<float, num_train>(reg_data.x_train),
        convert_array_to_vector<float, num_train>(reg_data.y_train_gt), "b");
    plt::named_plot("train_samples", convert_array_to_vector<float, num_train>(reg_data.x_train),
        convert_array_to_vector<float, num_train>(reg_data.y_train_noisy), "k");
    plt::named_plot("test_gt", convert_array_to_vector<float, num_test>(reg_data.x_test),
        convert_array_to_vector<float, num_test>(reg_data.y_test_gt), "r");
    plt::named_plot("test_samples", convert_array_to_vector<float, num_test>(reg_data.x_test),
        convert_array_to_vector<float, num_test>(reg_data.y_test_noisy), "k");
    plt::title("1D Toy dataset #1");
    plt::legend();
    plt::save("./toy_dataset1.png");

    // Updating and "training" the "full" GPr 
    // Note: limbo uses `Eigen`
    size_t dim_in = 1;
    GP_t vanilla_gpr = GP_t(dim_in, dim_in);
    for (int j=0; j < reg_data.x_train.size(); j++) {
        Eigen::VectorXd tmp_sample(1);
        tmp_sample(0) = reg_data.x_train[j];
        Eigen::VectorXd tmp_obs(1);
        tmp_obs(0) = reg_data.y_train_noisy[j];
        vanilla_gpr.add_sample(tmp_sample, tmp_obs);
    }
    

    std::vector<int> test_idxs;
    for (int ii=0; ii<reg_data.x_test.size(); ii++)
        test_idxs.push_back(ii);
    std::vector<float> test_samples;
    std::vector<float> test_gts;
    std::vector<float> test_predictions;
    std::vector<float> test_predictions_sigma;
    std::vector<float> test_predictions_plus_2s;
    std::vector<float> test_predictions_minus_2s;
    for (auto test_idx: test_idxs) {
        Eigen::VectorXd test_sample(1);
        test_sample(0) = reg_data.x_test[test_idx];
        Eigen::VectorXd test_gt(1);
        test_gt(0) = reg_data.y_test_noisy[test_idx];

        Eigen::VectorXd mu;
        float sigma;
        std::tie(mu, sigma) = vanilla_gpr.query(test_sample);
        std::cout << "The test sample (x, y)= " << reg_data.x_test[test_idx] << ", " <<
            reg_data.y_test_noisy[test_idx] << " with gpr pred (\\mu, \\sigma)= " <<
            mu <<", " << sigma << "\n";
    
        test_samples.push_back(static_cast<double>(test_sample(0)));
        test_gts.push_back(static_cast<double>(test_gt(0)));
        test_predictions.push_back(static_cast<double>(mu(0)));
        test_predictions_sigma.push_back(sigma);
        test_predictions_plus_2s.push_back(static_cast<double>(mu(0)) + 2*sigma);
        test_predictions_minus_2s.push_back(static_cast<double>(mu(0)) - 2*sigma);
    }
    
    
    // Plotting the GPr prediction on the test data
    plt::figure_size(1200, 780);
    plt::named_plot("test_gt", test_samples, test_gts, "-*b");
    plt::named_plot("gpr_mean", test_samples, test_predictions, "--g");
    // std::map<std::string, std::string> for_uncertainty_2s;
    // for_uncertainty_2s["alpha"] = "0.4";
    // for_uncertainty_2s["color"] = "grey";
    // for_uncertainty_2s["hatch"] = "-";
    // plt::fill_between(test_samples, test_predictions_minus_2s, test_predictions_plus_2s, for_uncertainty_2s);
    plt::named_plot("gpr_mean_plus_2s", test_samples, test_predictions_plus_2s, "--r");
    plt::named_plot("gpr_mean_minus_2s", test_samples, test_predictions_minus_2s, "--r");
    plt::title("1D Toy dataset #1");
    plt::legend();
    plt::save("./toy_dataset1_grp1.png");


    return 0;
}
