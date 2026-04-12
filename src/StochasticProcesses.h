//
// Created on 04-10-26
//

#ifndef HFT_STOCHASTICPROCESSES_H
#define HFT_STOCHASTICPROCESSES_H

#include <random>

/*
 * CIR (Cox Ingersoll Ross) variance process from the Heston model.
 * Euler Maruyama discretization with absorption at zero to prevent
 * negative variance when vol of vol is large relative to the
 * Feller condition (2*kappa*theta >= xi^2).
 */
class HestonProcess {
public:
    HestonProcess(double kappa, double theta, double xi, double v0);

    double step(double Z_v, double dt);
    [[nodiscard]] double variance() const;

private:
    double kappa_;
    double theta_;
    double xi_;
    double vt_;
};

/*
 * Ornstein Uhlenbeck process governing the annualized drift.
 * Mean reverts toward mu_bar at speed alpha, with noise scaled
 * by sigma_mu. Unlike the CIR process, drift is allowed to go
 * negative to model bear market regimes.
 */
class DriftProcess {
public:
    DriftProcess(double alpha, double mu_bar, double sigma_mu, double mu0);

    double step(double Z_mu, double dt);
    [[nodiscard]] double drift() const;

private:
    double alpha_;
    double mu_bar_;
    double sigma_mu_;
    double mu_t_;
};

/*
 * Kou double exponential jump process. Arrivals follow a Poisson
 * process (approximated as Bernoulli per time step). Jump sizes are
 * drawn from an asymmetric double exponential distribution where
 * upward jumps decay at rate eta1 and downward jumps at eta2,
 * with p controlling the probability of an upward jump.
 */
class KouJumpProcess {
public:
    static bool sampleArrival(double lambda, double dt, std::mt19937& rng);
    static double sampleSize(double p_up, double eta1, double eta2,
                             std::mt19937& rng);
};

#endif // HFT_STOCHASTICPROCESSES_H
