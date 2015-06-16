-- This Lua script demonstrates how to use the LuaFann module

-- It is based on the xor_train.c and xor_test.c example
-- programs distributed with FANN

local fann=require"fann"

-- Create a neural network, with 2 inputs, two hidden layer neurons,
-- and one output neuron
ann = fann.create_standard(3, 2, 2, 1)

-- Load training data from "xor.data"
train = fann.read_train_from_file("xor.data")

-- Set some parameters
ann:set_activation_steepness_hidden(1)
ann:set_activation_steepness_output(1)

ann:set_activation_function_hidden(fann.FANN_SIGMOID_SYMMETRIC)
ann:set_activation_function_output(fann.FANN_SIGMOID_SYMMETRIC)
ann:set_train_stop_function(fann.FANN_STOPFUNC_BIT)

ann:set_bit_fail_limit(0.01)

-- Initialise the weights based on the training data
ann:init_weights(train)

-- Train the network on the training data
ann:train_on_data(train, 500000, 1000, 0.001)

-- Test the network against the training data
mse = ann:test_data(train)
print("MSE: " .. mse)

-- Save the network to a file
ann:save("myxor.net")

-- For testing, recreate neural net from the file
ann = fann.create_from_file("myxor.net")
ann:print_connections()
ann:print_parameters()

-- Run the network through some samples to demonstrate its functionality
xor = ann:run(1, 1)
print("Result: " .. xor)
xor = ann:run(1, -1)
print("Result: " .. xor)
xor = ann:run(-1, -1)
print("Result: " .. xor)
xor = ann:run(-1, 1)
print("Result: " .. xor)

-- Now load some non-exact test data and test the NN
test = fann.read_train_from_file("xortest.data")
print("Test data read: " .. test:__tostring())
mse = ann:test_data(test)
print("MSE on test data: " .. mse)
