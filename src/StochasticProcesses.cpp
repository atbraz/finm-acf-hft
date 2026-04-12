//
// Created on 04-10-26
//

#include "StochasticProcesses.h"

#include <algorithm>
#include <cmath>

HestonProcess::HestonProcess(double kappa, double theta, double xi, double v0)
    : kappa_(kappa), theta_(theta), xi_(xi), vt_(v0) {}

double HestonProcess::step(double Z_v, double dt) {
    double sqrt_v = std::sqrt(std::max(vt_, 0.0));
    vt_ += kappa_ * (theta_ - vt_) * dt + xi_ * sqrt_v * std::sqrt(dt) * Z_v;
    vt_ = std::max(vt_, 0.0);
    return vt_;
}

double HestonProcess::variance() const { return vt_; }

DriftProcess::DriftProcess(double alpha, double mu_bar, double sigma_mu,
                           double mu0)
    : alpha_(alpha), mu_bar_(mu_bar), sigma_mu_(sigma_mu), mu_t_(mu0) {}

double DriftProcess::step(double Z_mu, double dt) {
    mu_t_ += alpha_ * (mu_bar_ - mu_t_) * dt + sigma_mu_ * std::sqrt(dt) * Z_mu;
    return mu_t_;
}

double DriftProcess::drift() const { return mu_t_; }

bool KouJumpProcess::sampleArrival(double lambda, double dt,
                                   std::mt19937& rng) {
    double prob = 1.0 - std::exp(-lambda * dt);
    std::bernoulli_distribution dist(prob);
    return dist(rng);
}

double KouJumpProcess::sampleSize(double p_up, double eta1, double eta2,
                                  std::mt19937& rng) {
    std::uniform_real_distribution<> uniform(0.0, 1.0);
    if (uniform(rng) < p_up) {
        std::exponential_distribution<> exp_dist(eta1);
        return exp_dist(rng);
    }
    std::exponential_distribution<> exp_dist(eta2);
    return -exp_dist(rng);
}
