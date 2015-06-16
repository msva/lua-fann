/******************************************************************
	LuaFann: Lua bindings for the Fast Artificial Neural Network Library
    Copyright (C) Vadim A. Misbakh-Soloviov
    Copyright (C) 2008-2009  Werner Stoop

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************/

/*
 *H Luafann
 *# Lua wrapper for the {*FANN*} neural network functions.\n
 *# \n
 *# To load Luafann as a Lua module use the {{require("fann")}} Lua construct.
 *# \n
 *# In the examples below, the variable {{ann}} refers to a neural network
 *# object instance created by {{fann.create_standard()}} or {{fann.create_from_file()}},
 *# and the variable {{train}} refers to a training set object instance created by
 *# {{fann.read_train_from_file()}}
 *-
 */
#include <stdio.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
/*
 * Compatibility for Lua 5.1.
 */

static void luaL_setfuncs (lua_State *l, const luaL_Reg *reg, int nup)
{
    int i;

    luaL_checkstack(l, nup, "too many upvalues");
    for (; reg->name != NULL; reg++) {
        for (i = 0; i < nup; i++)
            lua_pushvalue(l, -nup);
        lua_pushcclosure(l, reg->func, nup);
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);
}

#define lua_isinteger(L,l) lua_isnumber(L,l)
//#define luaL_newlibtable(L,l) lua_createtable(L,0,sizeof(l)/sizeof((l)[0]))
//#define luaL_newlib(L,l) (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))
#endif

#if LUA_VERSION_NUM > 501
/*
 * some syntactic-sugar macros removed after 5.1
 */
#define lua_tonumber(L,i) lua_tonumberx(L,(i),NULL)
#define lua_tointeger(L,i) lua_tointegerx(L,(i),NULL)
#define lua_tostring(L,i) lua_tolstring(L, (i), NULL)
#define lua_newtable(L) lua_createtable(L, 0, 0)
//#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))
#endif

#include <fann.h>

#define FANN_METATABLE "spil.fann"
#define FANN_TRAIN_METATABLE "spil.fanntrain"

/******************************************************************************
*h Neural Networks
*# These functions are used to create and configure neural networks
******************************************************************************/

/*! fann.create_standard(num_layers, neurons_1, neurons_2, ..., neurons_n)
 *# Creates a neural network with {{num_layers}}.\n
 *# The i'th layer will have {{neurons_i}} neurons (the function must thus have
 *# {{num_layers+1}} parameters in total).
 *x ann = fann.create_standard(3, 2, 3, 1)
 *-
 */
static int ann_create_standard(lua_State *L)
{
	struct fann **ann;
	int num_layers, i;
	unsigned int *layers;

	luaL_argcheck(L, lua_isinteger(L,1), 1, "First argument to fann.create_standard() must be an integer");

	num_layers = lua_tointeger(L, 1);
#ifdef FANN_VERBOSE
	printf("Creating neural net, %d layers\n", num_layers);
#endif

	if(num_layers < 1)
		luaL_error(L, "Neural network must have at least one layer");

	if(lua_gettop(L) < num_layers + 1)
		luaL_error(L, "Neural net has %d layers, so fann.open() must have %d parameters", num_layers, num_layers + 1);

	layers = lua_newuserdata(L, num_layers*(sizeof *layers));
	if(!layers)
		luaL_error(L, "out of memory");

	for(i = 0; i < num_layers; i++)
	{
		int n = luaL_checkinteger(L, i + 2);
		if(n < 1)
		{
			luaL_error(L, "Layer %d must have at least 1 neuron", i);
		}

#ifdef FANN_VERBOSE
		printf("Layer %d to have %d neurons\n", i, n);
#endif
		layers[i] = n;
	}

	ann = lua_newuserdata(L, sizeof *ann);

	luaL_getmetatable(L, FANN_METATABLE);
	lua_setmetatable(L, -2);

	*ann = fann_create_standard_array(num_layers, layers);
	if(!*ann)
	{
		luaL_error(L, "Unable to create neural network");
	}

	return 1;
}

/*! fann.create_sparse(connection_rate, num_layers, neurons_1, neurons_2, ..., neurons_n)
 *# Creates a neural network with {{num_layers}} that are not fully connected.\n
 *# The i'th layer will have {{neurons_i}} neurons (the function must thus have
 *# {{num_layers+1}} parameters in total).
 *x ann = fann.create_sparse(0.5, 3, 2, 3, 1)
 *-
 */
static int ann_create_sparse(lua_State *L)
{
	struct fann **ann;
	int num_layers, i;
	unsigned int *layers;
	float connection_rate;

	connection_rate = luaL_checknumber(L, 1);

	luaL_argcheck(L, lua_isinteger(L,2), 2, "Second argument to fann.create_sparse() must be an integer");

	num_layers = lua_tointeger(L, 2);
#ifdef FANN_VERBOSE
	printf("Creating neural net, %d layers\n", num_layers);
#endif

	if(num_layers < 1)
		luaL_error(L, "Neural network must have at least one layer");

	if(lua_gettop(L) < num_layers + 2)
		luaL_error(L, "Neural net has %d layers, so fann.create_sparse() must have %d parameters", num_layers, num_layers + 2);

	layers = lua_newuserdata(L, num_layers*(sizeof *layers));
	if(!layers)
		luaL_error(L, "out of memory");

	for(i = 0; i < num_layers; i++)
	{
		int n = luaL_checkinteger(L, i + 3);
		if(n < 1)
		{
			luaL_error(L, "Layer %d must have at least 1 neuron", i);
		}

#ifdef FANN_VERBOSE
		printf("Layer %d to have %d neurons\n", i, n);
#endif
		layers[i] = n;
	}

	ann = lua_newuserdata(L, sizeof *ann);

	luaL_getmetatable(L, FANN_METATABLE);
	lua_setmetatable(L, -2);

	*ann = fann_create_sparse_array(connection_rate, num_layers, layers);
	if(!*ann)
	{
		luaL_error(L, "Unable to create neural network");
	}

	return 1;
}

/*! fann.create_from_file(filename)
 *# Creates a neural network from a file.
 *x ann = fann.create_from_file("xor_float.net")
 *-
 */
static int ann_create_from_file(lua_State *L)
{
	struct fann **ann;
	const char *fname;

	luaL_argcheck(L, lua_isstring(L,1), 1, "Argument to fann.open_file() must be a string");

	fname = lua_tostring(L, 1);
#ifdef FANN_VERBOSE
	printf("Opening neural net '%s'\n", fname);
#endif

	ann = lua_newuserdata(L, sizeof *ann);

	luaL_getmetatable(L, FANN_METATABLE);
	lua_setmetatable(L, -2);

	*ann = fann_create_from_file(fname);
	if(!*ann)
		luaL_error(L, "Unable to create neural network from %s", fname);

	return 1;
}

/*! ann:__gc()
 *# Garbage collects the neural network.
 *-
 */
static int ann_close(lua_State *L)
{
	struct fann **ann;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

#ifdef FANN_VERBOSE
	printf("Collecting neural net\n");
#endif

	if(*ann)
	{
		fann_destroy(*ann);
		*ann = NULL;
	}

	return 0;
}

/*! ann:__tostring()
 *# Converts a neural net to a string for Lua's virtual machine
 *x print(ann)
 *-
 */
static int ann_tostring(lua_State *L)
{
	struct fann **ann;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	lua_pushfstring(L, "[[FANN neural network: %d %d %d]]", fann_get_num_input(*ann),
					fann_get_num_output(*ann), fann_get_total_neurons(*ann));
	return 1;
}
/*! ann:print_connections()
 *# Prints the connections in the neural network
 *x ann:print_connections()
 *-
 */
static int ann_print_connections(lua_State *L)
{
	struct fann **ann;
	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");
	fann_print_connections(*ann);
	return 0;
}

/*! ann:print_parameters()
 *# Prints the neural network's parameters
 *x ann:print_parameters()
 *-
 */
static int ann_print_parameters(lua_State *L)
{
	struct fann **ann;
	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");
	fann_print_parameters(*ann);
	return 0;
}

/*! ann:set_training_algorithm(function)
 *# Sets the training function for the neural network.\n
 *# Valid algorithms are {{fann.FANN_TRAIN_INCREMENTAL}},
 *# {{fann.FANN_TRAIN_BATCH}}, {{fann.FANN_TRAIN_RPROP}} or
 *# {{fann.FANN_TRAIN_QUICKPROP}}
 *x ann:set_training_algorithm(fann.FANN_TRAIN_QUICKPROP)
 *-
 */
static int ann_set_training_algorithm(lua_State *L)
{
	struct fann **ann;
	int alg;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	alg = luaL_checkinteger(L, 2);

	if(alg < FANN_TRAIN_INCREMENTAL || alg > FANN_TRAIN_QUICKPROP)
		luaL_error(L, "%d is not a valid algorithm", alg);

#ifdef FANN_VERBOSE
	printf("Setting training algorithm to %d\n", alg);
#endif

	fann_set_training_algorithm(*ann, alg);
	return 0;
}

/*! ann:get_training_algorithm()
 *# Retrieves the training algorithm:\n
 *# Valid algorithms are {{fann.FANN_TRAIN_INCREMENTAL}},
 *# {{fann.FANN_TRAIN_BATCH}}, {{fann.FANN_TRAIN_RPROP}} or
 *# {{fann.FANN_TRAIN_QUICKPROP}}
 *-
 */
static int ann_get_training_algorithm(lua_State *L)
{
	struct fann **ann;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	lua_pushinteger(L, fann_get_training_algorithm(*ann));
	return 1;
}

/*! ann:set_learning_rate(function)
 *# Sets the learning rate for the various training algorithms.
 *x ann:set_learning_rate(0.7)
 *-
 */
static int ann_set_learning_rate(lua_State *L)
{
	struct fann **ann;
	float rate;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	rate = luaL_checknumber(L, 2);

#ifdef FANN_VERBOSE
	printf("Setting learning rate to %g\n", rate);
#endif

	fann_set_learning_rate(*ann, rate);
	return 0;
}

/*! ann:get_learning_rate()
 *# Retrieves the learning rate of the training algorithm.
 *-
 */
static int ann_get_learning_rate(lua_State *L)
{
	struct fann **ann;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	lua_pushnumber(L, fann_get_learning_rate(*ann));
	return 1;
}

/*! ann:set_activation_function_hidden(function)
 *# Sets the activation function for the hidden layer neurons.
 *x ann:set_activation_function_hidden(fann.FANN_SIGMOID_SYMMETRIC)
 *-
 */
static int ann_set_activation_function_hidden(lua_State *L)
{
	struct fann **ann;
	int fun;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	fun = lua_tointeger(L, 2);

#ifdef FANN_VERBOSE
	printf("Setting hidden activation function to %d\n", fun);
#endif

	fann_set_activation_function_hidden(*ann, fun);
	return 0;
}

/*! ann:set_activation_function_output(function)
 *# Sets the activation function for the output neurons.
 *x ann:set_activation_function_output(fann.FANN_SIGMOID_SYMMETRIC)
 *-
 */
static int ann_set_activation_function_output(lua_State *L)
{
	struct fann **ann;
	int fun;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	fun = lua_tointeger(L, 2);

#ifdef FANN_VERBOSE
	printf("Setting output activation function to %d\n", fun);
#endif

	fann_set_activation_function_output(*ann, fun);
	return 0;
}

/*! ann:set_activation_steepness_hidden(function)
 *# Sets the steepness of the activation function for the hidden neurons.
 *x ann:set_activation_steepness_hidden(1)
 *-
 */
static int ann_set_activation_steepness_hidden(lua_State *L)
{
	struct fann **ann;
	fann_type steep;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	steep = lua_tonumber(L, 2);

#ifdef FANN_VERBOSE
	printf("Setting hidden layer activation steepness to %f\n", steep);
#endif

	fann_set_activation_steepness_hidden(*ann, steep);

	return 0;
}

/*! ann:set_activation_steepness_output(function)
 *# Sets the steepness of the activation function for the output neurons.
 *x ann:set_activation_steepness_output(1)
 *-
 */
static int ann_set_activation_steepness_output(lua_State *L)
{
	struct fann **ann;
	fann_type steep;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	steep = lua_tonumber(L, 2);

#ifdef FANN_VERBOSE
	printf("Setting output layer activation steepness to %f\n", steep);
#endif

	fann_set_activation_steepness_output(*ann, steep);

	return 0;
}

/*! ann:set_train_stop_function(function)
 *# Sets the training stop criteria.\n
 *# Valid values are either {{FANN_STOPFUNC_BIT}} or {{FANN_STOPFUNC_MSE}}
 *x ann:set_train_stop_function(fann.FANN_STOPFUNC_BIT)
 *-
 */
static int ann_set_train_stop_function(lua_State *L)
{
	struct fann **ann;
	int stop;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	stop = lua_tointeger(L, 2);

#ifdef FANN_VERBOSE
	printf("Setting training stop criteria to %d\n", stop);
#endif

	fann_set_train_stop_function(*ann, stop);

	return 0;
}

/*! ann:set_bit_fail_limit(limit)
 *# Sets the bit fail limit for training the neural net.
 *x ann:set_bit_fail_limit(0.01)
 *-
 */
static int ann_set_bit_fail_limit(lua_State *L)
{
	struct fann **ann;
	fann_type limit;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	limit = lua_tonumber(L, 2);

#ifdef FANN_VERBOSE
	printf("Setting bit fail limit to %f\n", limit);
#endif

	fann_set_bit_fail_limit(*ann, limit);

	return 0;
}

/*! ann:init_weights(train)
 *# Initializes the weights using Widrow and Nguyen's algorithm based on the
 *# given training data {{train}}.
 *x ann:init_weights(train)
 *-
 */
static int ann_init_weights(lua_State *L)
{
	struct fann **ann;
	struct fann_train_data **train;

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	train = luaL_checkudata(L, 2, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

#ifdef FANN_VERBOSE
	printf("Initialising weights to training data...\n");
#endif

	fann_init_weights(*ann, *train);

	return 0;
}

/*! ann:test_data(train)
 *# Runs the network through the training data in {{train}} and
 *# returns the MSE.
 *x mse = ann:test_data(train)
 *-
 */
static int ann_test_data(lua_State *L)
{
	struct fann **ann;
	struct fann_train_data **train;
	fann_type answer;

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	train = luaL_checkudata(L, 2, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

#ifdef FANN_VERBOSE
	printf("Testing network on training data...\n");
#endif

	answer = fann_test_data(*ann, *train);

	lua_pushnumber(L, answer);
	return 1;
}

/*! ann:run(input1, input2, ..., inputn)
 *# Evaluates the neural network for the given inputs.
 *x xor = ann:run(-1, 1)
 *-
 */
static int ann_run(lua_State *L)
{
	struct fann **ann;

	int nin, nout, i;
	fann_type *input, *output;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	nin = lua_gettop(L) - 1;
	if(nin != fann_get_num_input(*ann))
		luaL_error(L, "wrong number of inputs: expected %d, got %d", fann_get_num_input(*ann), nin);

	nout = fann_get_num_output(*ann);

#ifdef FANN_VERBOSE
	printf("Evaluating neural net: %d inputs, %d outputs\n", nin, nout);
#endif

	input = lua_newuserdata(L, nin*(sizeof *input));
	if(!input)
		luaL_error(L, "out of memory");

	for(i = 0; i < nin; i++)
	{
		input[i] = luaL_checknumber(L, i + 2);
#ifdef FANN_VERBOSE
		printf("Input %d's value is %f\n", i, input[i]);
#endif
	}

	output = fann_run(*ann, input);
	for(i = 0; i < nout; i++)
	{
#ifdef FANN_VERBOSE
	printf("Output %d's value is %f\n", i, output[i]);
#endif
		lua_pushnumber(L, output[i]);
	}

	return nout;
}

/*! ann:save(file)
 *# Saves a neural network to a file named {{file}}
 *x ann:save("xor_float.net")
 *-
 */
static int ann_save(lua_State *L)
{
	struct fann **ann;
	const char *fname;

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	fname = lua_tostring(L, 2);
#ifdef FANN_VERBOSE
	printf("Saving neural net to \"%s\"\n", fname);
#endif

	fann_save(*ann, fname);
	return 0;
}

/******************************************************************************
*h Training Sets
*# These functions are used to create and manage training sets
******************************************************************************/

/*! fann.read_train_from_file(filename)
 *# Creates a training object by reading a training data file.
 *x train = fann.read_train_from_file("xor.data")
 *-
 */
static int ann_read_train_from_file(lua_State *L)
{
	struct fann_train_data **train;
	const char *fname;

	luaL_argcheck(L, lua_isstring(L,1), 1, "Argument to fann.open_file() must be a string");

	fname = lua_tostring(L, 1);
#ifdef FANN_VERBOSE
	printf("Opening training data from file '%s'\n", fname);
#endif

	train = lua_newuserdata(L, sizeof *train);

	luaL_getmetatable(L, FANN_TRAIN_METATABLE);
	lua_setmetatable(L, -2);

	*train = fann_read_train_from_file(fname);
	if(!*train)
		luaL_error(L, "Unable to read train data from %s", fname);

	return 1;
}

/*! train:__gc()
 *# Garbage collects training data.
 *-
 */
static int ann_train_close(lua_State *L)
{
	struct fann_train_data **train;

	train = luaL_checkudata(L, 1, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

#ifdef FANN_VERBOSE
	printf("Closing training data\n");
#endif

	if(*train)
	{
		fann_destroy_train(*train);
		*train = NULL;
	}

	return 0;
}

/*! train:__tostring()
 *# Converts training data to a string for Lua's virtual machine
 *x print(train)
 *-
 */
static int ann_train_tostring(lua_State *L)
{
	struct fann_train_data **train;

	train = luaL_checkudata(L, 1, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

	lua_pushfstring(L, "[[FANN training data]]");
	return 1;
}

/*! ann:train_on_file(file, max_epochs, epochs_between_reports, desired_error)
 *# Trains the neural network on the data in the file {{file}}, for up to
 *# {{max_epochs}} epochs, reporting every {{epochs_between_reports}}.
 *# Training stops when the error reaches {{desired_error}}
 *x ann:train_on_file("xor.data", 500000, 1000, 0.001)
 *-
 */
static int ann_train_on_file(lua_State *L)
{
	struct fann **ann;
	const char *fname;
	int max_epochs, epochs_between_reports;
	float desired_error;

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	if(lua_gettop(L) < 5)
		luaL_error(L, "insufficient parameters");

	fname = lua_tostring(L, 2);
	max_epochs = lua_tointeger(L, 3);
	epochs_between_reports = lua_tointeger(L, 4);
	desired_error = lua_tonumber(L, 5);

#ifdef FANN_VERBOSE
	printf("Training on file \"%s\" for up to %d epochs...\n", fname, max_epochs);
#endif

	fann_train_on_file(*ann, fname, max_epochs, epochs_between_reports, desired_error);
	return 0;
}

/*! ann:train_on_data(train, max_epochs, epochs_between_reports, desired_error)
 *# Trains the neural network on the data in {{train}}, for up to
 *# {{max_epochs}} epochs, reporting every {{epochs_between_reports}}.
 *# Training stops when the error reaches {{desired_error}}
 *x ann:train_on_data(train, 500000, 1000, 0.001)
 *-
 */
static int ann_train_on_data(lua_State *L)
{
	struct fann **ann;
	struct fann_train_data **train;
	int max_epochs, epochs_between_reports;
	float desired_error;

	if(lua_gettop(L) < 5)
		luaL_error(L, "insufficient parameters");

	ann = luaL_checkudata(L, 1, FANN_METATABLE);
	luaL_argcheck(L, ann != NULL, 1, "'neural net' expected");

	train = luaL_checkudata(L, 2, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

	max_epochs = lua_tointeger(L, 3);
	epochs_between_reports = lua_tointeger(L, 4);
	desired_error = lua_tonumber(L, 5);

#ifdef FANN_VERBOSE
	printf("Training on data for up to %d epochs...\n", max_epochs);
#endif

	fann_train_on_data(*ann, *train, max_epochs, epochs_between_reports, desired_error);
	return 0;
}

/*! train:save(filename)
 *# Saves training data to a specified file
 *x train:save("train.data")
 *-
 */
static int ann_save_train(lua_State *L)
{
	struct fann_train_data **train;
	const char *fname;

	if(lua_gettop(L) < 2)
		luaL_error(L, "insufficient parameters");

	train = luaL_checkudata(L, 1, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

	fname = lua_tostring(L, 2);

#ifdef FANN_VERBOSE
	printf("Saving training data to %s\n", fname);
#endif
	fann_save_train(*train, fname);
	return 0;
}

/*! train:scale_input(min, max)
 *# Scales the inputs of training data  to the new range [{{min}}-{{max}}]
 *x
 *-
 */
static int ann_train_scale_input(lua_State *L)
{
	struct fann_train_data **train;
	fann_type min, max;

	train = luaL_checkudata(L, 1, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

	min = luaL_checknumber(L, 2);
	max = luaL_checknumber(L, 3);

#ifdef FANN_VERBOSE
	printf("Scaling training data inputs to %g - %g\n", (double)min, (double)max);
#endif

	fann_scale_input_train_data(*train, min, max);

	return 0;
}

/*! train:scale_output(min, max)
 *# Scales the outputs of training data to the new range [{{min}}-{{max}}]
 *x
 *-
 */
static int ann_train_scale_output(lua_State *L)
{
	struct fann_train_data **train;
	fann_type min, max;

	train = luaL_checkudata(L, 1, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

	min = luaL_checknumber(L, 2);
	max = luaL_checknumber(L, 3);

#ifdef FANN_VERBOSE
	printf("Scaling training data outputs to %g - %g\n", (double)min, (double)max);
#endif
	fann_scale_output_train_data(*train, min, max);

	return 0;
}

/*! train:scale(min, max)
 *# Scales the inputs and outputs of training data to the new range [{{min}}-{{max}}]
 *x
 *-
 */
static int ann_train_scale(lua_State *L)
{
	struct fann_train_data **train;
	fann_type min, max;

	train = luaL_checkudata(L, 1, FANN_TRAIN_METATABLE);
	luaL_argcheck(L, train != NULL, 1, "'training data' expected");

	min = luaL_checknumber(L, 2);
	max = luaL_checknumber(L, 3);

#ifdef FANN_VERBOSE
	printf("Scaling training data to %g - %g\n", (double)min, (double)max);
#endif

	fann_scale_train_data(*train, min, max);

	return 0;
}

/* ************************************************************************** */

/* Members of FANN objects
 * __gc is a Lua metamethod called when the object is garbage collected
 *    (see PIL chapter 29)
 * __tostring is a Lua metamethod to convert the object to a string
 */
static const struct luaL_Reg fann_lib_members[] = {
  {"__gc", ann_close},
  {"__tostring", ann_tostring},
  {"print_connections", ann_print_connections},
  {"print_parameters", ann_print_parameters},
  {"set_training_algorithm", ann_set_training_algorithm},
  {"get_training_algorithm", ann_get_training_algorithm},
  {"set_learning_rate", ann_set_learning_rate},
  {"get_learning_rate", ann_get_learning_rate},
  {"set_activation_function_hidden", ann_set_activation_function_hidden},
  {"set_activation_function_output", ann_set_activation_function_output},
  {"set_activation_steepness_hidden", ann_set_activation_steepness_hidden},
  {"set_activation_steepness_output", ann_set_activation_steepness_output},
  {"set_train_stop_function", ann_set_train_stop_function},
  {"set_bit_fail_limit", ann_set_bit_fail_limit},
  {"train_on_file", ann_train_on_file},
  {"train_on_data", ann_train_on_data},
  {"init_weights", ann_init_weights},
  {"test_data", ann_test_data},
  {"save", ann_save},
  {"run", ann_run},
  {NULL, NULL}
};

static const struct luaL_Reg fann_train_lib_members[] = {
  {"__gc", ann_train_close},
  {"__tostring", ann_train_tostring},
  {"save", ann_save_train},
  {"scale_input", ann_train_scale_input},
  {"scale_output", ann_train_scale_output},
  {"scale", ann_train_scale},
  {NULL, NULL}
};

struct iglobal { char *name; int value; };

/*h Constants
 *# The {{fann}} class also contains several variables that reflect the constants defined
 *# for FANN in {{fann_data.h}}, as listed below:\n
 */
/* Members of the FANN class */
static const struct luaL_Reg fann_lib[] = {
  {"create_standard", ann_create_standard},
  {"create_sparse", ann_create_sparse},
  {"create_from_file", ann_create_from_file},
  {"read_train_from_file", ann_read_train_from_file},
  {NULL, NULL}
};

static const struct iglobal fann_activation_functions [] = {
	/*# The following is a list of all activation functions in FANN.
	 *{
 	 ** {{fann.FANN_LINEAR}}
	 */
	{"FANN_LINEAR", 					FANN_LINEAR},
	/** {{fann.FANN_THRESHOLD}}
	 */
	{"FANN_THRESHOLD", 					FANN_THRESHOLD},
	/** {{fann.FANN_THRESHOLD_SYMMETRIC}}
	 */
	{"FANN_THRESHOLD_SYMMETRIC", 		FANN_THRESHOLD_SYMMETRIC},
	/** {{fann.FANN_SIGMOID}}
	 */
	{"FANN_SIGMOID", 					FANN_SIGMOID},
	/** {{fann.FANN_SIGMOID_STEPWISE}}
	 */
	{"FANN_SIGMOID_STEPWISE", 			FANN_SIGMOID_STEPWISE},
	/** {{fann.FANN_SIGMOID_SYMMETRIC}}
	 */
	{"FANN_SIGMOID_SYMMETRIC", 			FANN_SIGMOID_SYMMETRIC},
	/** {{fann.FANN_SIGMOID_SYMMETRIC_STEPWISE}}
	 */
	{"FANN_SIGMOID_SYMMETRIC_STEPWISE", FANN_SIGMOID_SYMMETRIC_STEPWISE},
	/** {{fann.FANN_GAUSSIAN}}
	 */
	{"FANN_GAUSSIAN", 					FANN_GAUSSIAN},
	/** {{fann.FANN_GAUSSIAN_SYMMETRIC}}
	 */
	{"FANN_GAUSSIAN_SYMMETRIC", 		FANN_GAUSSIAN_SYMMETRIC},
	/** {{fann.FANN_ELLIOT}}
	 */
	{"FANN_ELLIOT", 					FANN_ELLIOT},
	/** {{fann.FANN_ELLIOT_SYMMETRIC}}
	 */
	{"FANN_ELLIOT_SYMMETRIC", 			FANN_ELLIOT_SYMMETRIC},
	/** {{fann.FANN_LINEAR_PIECE}}
	 */
	{"FANN_LINEAR_PIECE", 				FANN_LINEAR_PIECE	},
	/** {{fann.FANN_LINEAR_PIECE_SYMMETRIC}}
	 */
	{"FANN_LINEAR_PIECE_SYMMETRIC", 	FANN_LINEAR_PIECE_SYMMETRIC},
	/*}*/

	/*# The following is a list of the training algorithms that can be used.
	 *{
     */
	/** {{fann.FANN_TRAIN_INCREMENTAL}}
	 */
	{"FANN_TRAIN_INCREMENTAL", 	FANN_TRAIN_INCREMENTAL},
	/** {{fann.FANN_TRAIN_BATCH}}
	 */
	{"FANN_TRAIN_BATCH", 	FANN_TRAIN_BATCH},
	/** {{fann.FANN_TRAIN_RPROP}}
	 */
	{"FANN_TRAIN_RPROP", 	FANN_TRAIN_RPROP},
	/** {{fann.FANN_TRAIN_QUICKPROP}}
	 */
	{"FANN_TRAIN_QUICKPROP", 	FANN_TRAIN_QUICKPROP},

	/* FANN 2.1 defines some more, but I'm sticking with 2.0 for now */
	/*}*/

	/*# The following is a list of the stop criteria used during training
	 *{
	 ** {{fann.FANN_STOPFUNC_MSE}}
	 */
	{"FANN_STOPFUNC_MSE", 	FANN_STOPFUNC_MSE},
	/** {{fann.FANN_STOPFUNC_BIT}}
	 */
	{"FANN_STOPFUNC_BIT", 	FANN_STOPFUNC_BIT},
	/*}
	 *-
	 */
	{NULL, 0}
};

LUALIB_API int luaopen_fann(lua_State *L)
{
	const struct iglobal *i;

	assert(L);

#ifdef FANN_VERBOSE
	printf("Registering FANN library\n");
#endif

	/* Refer to PIL 28.3 for an explaination:
	 * These calls all set the metatables of
	 * FANN objects,
 	 */
	luaL_newmetatable(L, FANN_METATABLE);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);
	luaL_setfuncs(L, fann_lib_members, 0);

	luaL_newmetatable(L, FANN_TRAIN_METATABLE);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);
	luaL_setfuncs(L, fann_train_lib_members, 0);

	lua_newtable(L);
	luaL_setfuncs(L, fann_lib, 0);

	/* The table 'fann' is now still on the top of the
		stack, so register some globals with it...
	*/
	for(i = fann_activation_functions; i->name; i++)
	{
#ifdef FANN_VERBOSE
		printf("Setting fann.%s to %d\n", i->name, i->value);
#endif
		lua_pushstring(L, i->name);
		lua_pushinteger(L, i->value);
		lua_settable(L, -3);
	}

	return 1;
}
