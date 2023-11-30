#include "PyThreadStateLock.h"
#include <QDebug>
#include <iostream>

void PythonInit()
{
    if (!Py_IsInitialized())
    {
        //1.初始化Python解释器，这是调用操作的第一步
        Py_Initialize();
        if (!Py_IsInitialized()) {
            qDebug("Initial Python failed!");
//            emit failed();
        }
        else {

            //执行单句Python语句，用于给出调用模块的路径，否则将无法找到相应的调用模块
            // 初始化线程支持
            PyEval_InitThreads();

            PyRun_SimpleString("import sys");
            PyRun_SimpleString("sys.path.append('C:/postgraduate/LVPredictor-software/LVPredictor/')"); // 设置绝对路径

            // 启动子线程前执行，为了释放PyEval_InitThreads获得的全局锁，否则子线程可能无法获取到全局锁。
            PyEval_ReleaseThread(PyThreadState_Get());
            qDebug("Initial Python Success!");

        }
    }

}

void testPython(){
    class PyThreadStateLock PyThreadLock;//获取全局锁
    PyObject * pModule = NULL;
    PyObject * pFunc = NULL;

    pModule = PyImport_ImportModule("lsl");//调用的Python文件名  py文件放置exe同级

    if (pModule == NULL)
    {
        PyErr_Print();
        std::cout << "PyImport_ImportModule Fail!" << std::endl;
        return;
    }

    // 设置调用的函数名
    pFunc = PyObject_GetAttrString(pModule, "received_data");

    // 转换输入数据，将字典输入
    PyObject* dict = PyDict_New();
//    for(auto data : inputData){
//        PyDict_SetItem(dict, Py_BuildValue("s", data.first.c_str()), Py_BuildValue("d", data.second)); // 这里要注意"s"表示c类型的字符串，所以要加c_str()否则报错
//    }

    // 创建参数对象
    PyObject* args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, dict); // 0表示数据dict在参数args中的位置索引

    PyObject *pReturn = NULL; //返回值
    pReturn = PyEval_CallObject(pFunc, args); //调用函数

    // 转换返回值格式
    double re;
    PyArg_Parse(pReturn, "d", &re);
    std::cout<<"result: "<<re<<std::endl;


    // Py_Finalize();//注意这一句不能要，否则会运行崩溃
}

