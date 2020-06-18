
/*  
 *  beam_wrap.cpp
 *
 *  This code is a wrapper function for the python code provided to calculate 
 *  the beamforming coefficients.
 *  
 *  As of right now this code takes data stored in the txt files and passes it to 
 *  the python beamform function which then generates the coefficients. Ideally 
 *  we will get to a point where we can call a c++ function which pulls data
 *  from a shared meory location and also stores data in a designed I/O location,
 *  or in a way where the filters can be filled with the coefficients.
 * 
 *  Kanad Sarkar
 */

/*
 *  Using the C-Python API:
 *   
 *  This API is finicky because things can change drasticlally given the version,
 *  using a different version of numpy or python may or not give you the same results.
 *  Normally this API is used to embedd C functions within Python for performance use 
 *  but we are doing the opposite.
 *
 *  So far I've used this with Python 3.7 and Python 3.7m, the m version represent the
 *  pymalloc version of python which is beneficial for using c with python, but I think 
 *  there will not be that much difference, because we don't actually emebed C within 
 *  the Python.
 *
 *  The numpy verison is 1.7 and I would suggest looking at the Numpy C API if you want to 
 *  reap the benefits of a future verison of the Numpy-c API
 *
 *  An annoying part of working with this API, especially across platforms, is the 
 *  compiling & linker flags. 
 *  
 *  To get the flgas to use just python type in these two things.
 *  pythonX.Y-config --cflags
 *  pythonX.Y-config --ldflags
 *
 *  If you want to then use numpy objects in any capacity in c/c++:
 *  open python terminal
 *  import numpy as np
 *  np.get_include()
 *  take the path printed and add a I/ in front of it, then add it to the compile flags
 *
 */

#include <iostream>
#include <fstream> 
#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#define NUM_COEF 4096
#define NUM_OUTPUT 1
#define NUM_SOURCES 8
#define BINAURAL 2
#define NOISE_LEN 1440001

using namespace std;

int main()
{
    setenv("PYTHONPATH",".",1); // Required to find the proper module
    float * target_ir = new float [NUM_SOURCES * NUM_COEF];
    float * noise = new float [NUM_SOURCES * NOISE_LEN]; 
    double filter [NUM_COEF][NUM_SOURCES][BINAURAL][NUM_OUTPUT]; // python float is a c double, filter coefficients
    npy_intp tar_dims[3] = { NUM_COEF, NUM_SOURCES, NUM_OUTPUT }; //target irs will be three dimensions 
    npy_intp noise_dims[2] = { NOISE_LEN, NUM_SOURCES }; // noise will be in 2 dimensions
    PyObject* coefficient; // pointer for the coefficent array in python

    /* Generating the array holding the target data for txt*/
    ifstream target_file;
    target_file.open("target_ir.txt");
    if (!target_file) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }
     for (int i = 0; i < NUM_COEF; i++) {
        for (int j = 0; j < NUM_SOURCES; j++) {
            target_file >> target_ir[NUM_SOURCES * i + j];
        }
    }   
    target_file.close();

    /* Generating the array holding the noise data for txt*/
    ifstream noise_file;
    noise_file.open("noise.txt");
    if (!noise_file) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }  
    for (int i = 0; i < NOISE_LEN; i++) {
        for (int j = 0; j < NUM_SOURCES; j++) {
            noise_file >> noise[NUM_SOURCES * i + j];
        }
    }   
    noise_file.close();

   
    
    // Initialize the C-Python API and tell the c code to the python code
    Py_Initialize();
    
    PyObject * pName  = PyUnicode_FromString("ez_beamform");
    if(pName == NULL)
    {
        cout << "unable to find the python file" << endl;
        exit(1);
    }
    
    PyObject* pModule = PyImport_Import(pName);
    if(pModule == NULL)
    {
        cout << "unable to find the python module, set the PYTHONPATH" << endl;
        exit(1);
    }


    PyObject* pDict = PyModule_GetDict(pModule);
    if(pDict == NULL)
    {
        cout << "unable to find the python dict, idk what you did" << endl;
        exit(1);
    }

    
    import_array(); // This function is required to use numpy arrays with the C-Python API 

    // These lines create Pythonic arrays off of our C arrays and then creates our args for the py func
    PyObject* targ_array = PyArray_SimpleNewFromData(3, tar_dims, NPY_FLOAT, target_ir);
    PyObject* noise_array = PyArray_SimpleNewFromData(2, noise_dims, NPY_FLOAT, noise); 
    PyObject* pArgs = PyTuple_New (2);
    PyTuple_SetItem (pArgs, 0, targ_array);
    PyTuple_SetItem (pArgs, 1, noise_array);

    // Get the function name, and then run it and copy it over to a c array 
    PyObject* pFunc = PyDict_GetItemString (pDict, (char*)"beamformer_binaural"); 
    if (PyCallable_Check (pFunc))
    {
        coefficient = PyObject_CallObject(pFunc, pArgs);
        for (int i = 0; i < NUM_COEF; i++)
        {
            for (int j = 0; j < NUM_SOURCES; j++)
            {
                for (int k = 0; k < BINAURAL; k++)
                {
                    for (int l = 0; l < NUM_OUTPUT; l++)
                    {
                        filter[i][j][k][l] = (*((double *)PyArray_GETPTR4((PyArrayObject *)coefficient, i,j,k,l))); // Used to obtain a pointer within the Python Array Object
                    }
                }
            }
        }
        Py_DECREF(coefficient);
    } 
    else
    {
        cout << "Function is not callable !" << endl;
    }
    
    // Required for reference counting, will seg fault if misused and is very hard to debug
    Py_DECREF(pName);
    Py_DECREF (targ_array);  
    Py_DECREF (noise_array);
    Py_DECREF (pArgs); 
    Py_DECREF (pModule);
    Py_DECREF (pDict);
    Py_DECREF (pFunc);
    
    //clean up, still need to verify that it is not leaking
    delete[] noise;
    delete[](target_ir);
    
    /*// This is for verifying that my local array has the python output
    cout.precision(16);
    cout << filter[1025][5][1][0] << endl;
    */
    
    Py_Finalize(); // Required for the C-Python API
    
    return 0;

}
