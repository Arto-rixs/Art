#include <iostream>
#include <eigen3/Eigen/Dense>
#include <cmath>

// Activation function (Sigmoid)
double sigmoid(double x) {
    return 1 / (1 + exp(-x));
}

// Derivative of the sigmoid function
double sigmoid_derivative(double x) {
    return x * (1 - x);
}

int main() {
    // Input dataset
    Eigen::MatrixXd X(63, 4);
    X << 87.36, 88.18, 85.68, 86.26,
         86.15, 87.56, 84.53, 86.85,
         86.70, 89.02, 85.40, 88.87,
         88.98, 90.18, 87.59, 89.52,
         89.60, 90.14, 87.74, 88.79,
         88.36, 91.43, 87.33, 90.36,
         90.32, 93.43, 89.68, 93.17,
         93.25, 94.90, 92.25, 94.74,
         94.64, 95.37, 93.57, 94.65,
         94.70, 97.93, 94.40, 97.66,
         97.65, 98.68, 96.70, 97.35,
         97.49, 97.92, 94.30, 95.21,
         95.50, 95.89, 93.21, 93.36,
         93.22, 95.27, 92.30, 95.10,
         95.18, 96.20, 92.90, 93.42,
         93.49, 94.17, 92.03, 93.19,
         93.15, 93.95, 90.37, 90.97,
         90.91, 92.15, 90.56, 91.70,
         91.77, 95.07, 91.67, 93.64,
         93.55, 94.15, 92.00, 93.80,
         93.85, 94.50, 92.08, 93.54,
         93.63, 94.68, 92.50, 93.98,
         93.97, 96.13, 92.91, 96.08,
         96.14, 97.36, 95.22, 96.92,
         96.99, 97.28, 95.24, 95.71,
         93.88, 96.09, 92.60, 95.45,
         95.40, 96.88, 95.03, 96.76,
         96.80, 97.14, 94.84, 95.99,
         96.03, 99.45, 95.10, 99.21,
         99.26, 99.85, 97.56, 98.49,
         98.66, 98.84, 94.35, 94.67,
         94.65, 95.33, 92.96, 94.80,
         94.90, 97.28, 94.58, 96.90,
         96.80, 97.17, 95.25, 96.33,
         96.25, 96.25, 92.76, 93.19,
         93.30, 96.00, 92.81, 93.55,
         93.39, 93.86, 92.06, 92.35,
         92.24, 92.24, 88.12, 89.26,
         89.27, 89.53, 84.97, 85.22,
         85.15, 90.00, 84.17, 89.67,
         89.55, 90.20, 86.17, 86.56,
         86.59, 87.35, 85.56, 86.18,
         86.15, 87.80, 85.92, 86.90,
         86.76, 86.93, 82.16, 83.37,
         83.34, 86.29, 83.28, 85.07,
         85.31, 85.73, 83.54, 85.48,
         88.14, 90.33, 87.86, 89.90,
         89.90, 89.91, 88.25, 89.10,
         89.06, 90.63, 88.00, 89.08,
         89.00, 89.15, 85.38, 85.65,
         85.74, 85.79, 82.40, 83.13,
         83.00, 83.05, 81.20, 81.58,
         81.43, 81.95, 80.04, 80.21,
         80.30, 80.97, 78.67, 80.86,
         80.91, 82.53, 80.34, 82.09,
         82.20, 83.11, 81.56, 82.77,
         82.85, 83.85, 82.55, 83.33,
         83.20, 83.25, 80.35, 80.64,
         80.60, 81.49, 80.00, 80.88,
         82.73, 81.87, 80.03, 81.33,
         81.27, 83.45, 79.49, 83.12,
         83.21, 84.85, 82.83, 83.07,
         83.01, 84.57, 82.25, 84.48,
         84.41, 84.81, 84.03, 84.60,
         84.65, 85.75, 83.86, 85.55,
         85.50, 86.16, 82.59, 82.90,
         82.99, 83.98, 81.76, 82.21,
         85.51, 87.75, 85.17, 87.46,
         87.60, 89.10, 87.59, 87.98;

    // Output dataset
    Eigen::MatrixXd y(63, 1);
    y << 86.15,
         86.90,
         88.36,
         89.60,
         88.79,
         90.32,
         93.25,
         94.64,
         94.70,
         97.65,
         97.49,
         95.50,
         93.22,
         95.18,
         93.49,
         93.15,
         90.91,
         91.77,
         93.55,
         93.85,
         93.63,
         93.97,
         96.14,
         96.99,
         93.88,
         95.40,
         96.80,
         96.03,
         99.26,
         98.66,
         94.65,
         94.90,
         96.80,
         96.25,
         93.30,
         93.39,
         92.24,
         89.27,
         85.15,
         89.55,
         86.59,
         86.15,
         86.76,
         83.34,
         85.31,
         88.14,
         89.90,
         89.06,
         89.00,
         85.74,
         83.00,
         81.43,
         80.30,
         80.91,
         82.20,
         82.85,
         83.20,
         80.60,
         82.73,
         81.27,
         83.21,
         83.01,
         84.41,
         84.65,
         85.50,
         82.99,
         85.51,
         87.60;

    // Scale the input and output datasets
    Eigen::MatrixXd X_normalized = X.colwise() - X.rowwise().mean();
    Eigen::MatrixXd y_normalized = y.colwise() - y.rowwise().mean();

    // Learning rate
    double alpha = 0.01;

    // Number of epochs
    int epochs = 1000;

    // Number of input neurons
    int input_neurons = 4;

    // Number of hidden neurons
    int hidden_neurons = 3;

    // Number of output neurons
    int output_neurons = 1;

    // Initialize weights
    Eigen::MatrixXd input_hidden_weights = Eigen::MatrixXd::Random(input_neurons, hidden_neurons);
    Eigen::MatrixXd hidden_output_weights = Eigen::MatrixXd::Random(hidden_neurons, output_neurons);

    // Initialize biases
    Eigen::MatrixXd hidden_biases = Eigen::MatrixXd::Random(1, hidden_neurons);
    Eigen::MatrixXd output_biases = Eigen::MatrixXd::Random(1, output_neurons);

    // Training the neural network
    for (int i = 0; i < epochs; ++i) {
        // Forward pass
        Eigen::MatrixXd hidden_layer_input = X_normalized * input_hidden_weights;
        Eigen::MatrixXd hidden_layer_output = hidden_layer_input.unaryExpr(&sigmoid);
        Eigen::MatrixXd output_layer_input = hidden_layer_output * hidden_output_weights;
        Eigen::MatrixXd predicted_output = output_layer_input.unaryExpr(&sigmoid);

        // Backpropagation
        Eigen::MatrixXd error = y_normalized - predicted_output;
        Eigen::MatrixXd output_delta = error.cwiseProduct(output_layer_input.unaryExpr(&sigmoid_derivative));
        Eigen::MatrixXd hidden_error = output_delta * hidden_output_weights.transpose();
        Eigen::MatrixXd hidden_delta = hidden_error.cwiseProduct(hidden_layer_input.unaryExpr(&sigmoid_derivative));

        // Update weights and biases
        hidden_output_weights += hidden_layer_output.transpose() * output_delta * alpha;
        input_hidden_weights += X_normalized.transpose() * hidden_delta * alpha;
        output_biases += output_delta.rowwise().sum() * alpha;
        hidden_biases += hidden_delta.rowwise().sum() * alpha;
    }

    // Predict
    Eigen::MatrixXd new_data(1, 4);
    new_data << 87.60, 89.10, 87.59, 87.98;
    Eigen::MatrixXd new_data_normalized = new_data.colwise() - X.rowwise().mean();
    Eigen::MatrixXd hidden_layer_input_new = new_data_normalized * input_hidden_weights;
    Eigen::MatrixXd hidden_layer_output_new = hidden_layer_input_new.unaryExpr(&sigmoid);
    Eigen::MatrixXd output_layer_input_new = hidden_layer_output_new * hidden_output_weights;
    Eigen::MatrixXd predicted_output_new = output_layer_input_new.unaryExpr(&sigmoid);

    // Scale back the predicted output
    predicted_output_new = predicted_output_new.colwise() + y.rowwise().mean();

    std::cout << "Predicted Close Price: $" << predicted_output_new(0, 0) << std::endl;

    return 0;
}
