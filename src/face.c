
#if 0
1、包含Python.h头文件，以便使用Python API。
2、使用void Py_Initialize()初始化Python解释器，
3、使用PyObject *PyImport_ImportModule(const char *name)和PyObject
*PyObject_GetAttrString(PyObject *o, const char *attr_name)获取sys.path对象，并利用
int PyList_Append(PyObject *list, PyObject *item)将当前路径.添加到sys.path中，以便加载
当前的Python模块(Python文件即python模块)。
4、使用PyObject *PyImport_ImportModule(const char *name)函数导入Python模块，并检查是否
有错误。
5、使用PyObject *PyObject_GetAttrString(PyObject *o, const char *attr_name)函数获取
Python函数对象，并检查是否可调用。
6、使用PyObject *PyObject_CallObject(PyObject *callable, PyObject *args)函数调用
Python函数，并获取返回值。
7、使用void Py_DECREF(PyObject *o)函数释放所有引用的Python对象。
8、结束时调用void Py_Finalize()函数关闭Python解释器。
相关的函数参数说明参考网站（网站左上角输入函数名即可开始搜索）：
https://docs.python.org/zh-cn/3/c-api/import.html
#endif

#include <string.h>
#include <Python.h>
//#include "face.h"

#define WGET_CMD "wget http://127.0.0.1:8080/?action=snapshot -O /tmp/SearchFace.jpg"
#define SEARCHFACE_FILE "/tmp/SearchFace.jpg"


// 静态变量：标记解释器是否已初始化，避免重复调用 Py_Initialize
static int is_initialized = 0;

int face_init(void) {
    int status = 0;

    // 检查是否已初始化，避免重复调用 Py_Initialize（关键！）
    if (is_initialized) {
        return 0; // 已初始化，直接返回成功
    }

    // 初始化 Python 解释器
    Py_Initialize();
    // 检查初始化是否成功（虽然 Py_Initialize 无返回值，但可通过 Py_IsInitialized 判断）
    if (!Py_IsInitialized()) {
        printf("Error: Py_Initialize failed\n");
        return -1;
    }

    // 导入 sys 模块（获取局部引用）
    PyObject *sys = PyImport_ImportModule("sys");
    if (!sys) {
        PyErr_Print();
        printf("Error: failed to load sys module\n");
        status = -1;
        goto FAILED_SYS; // sys 为 NULL，无需释放，直接跳转到最终清理
    }

    // 获取 sys.path（列表对象，局部引用）
    PyObject *path = PyObject_GetAttrString(sys, "path");
    if (!path) {
        PyErr_Print();
        printf("Error: failed to get sys.path\n");
        status = -1;
        goto FAILED_PATH; // 释放已获取的 sys
    }

    // 创建当前目录字符串对象（临时引用）
    PyObject *cwd = PyUnicode_FromString(".");
    if (!cwd) { // 检查字符串创建是否成功（内存不足可能失败）
        PyErr_Print();
        printf("Error: failed to create cwd string\n");
        status = -1;
        goto FAILED_CWD; // 释放 path 和 sys
    }

    // 将当前目录添加到 sys.path
    if (PyList_Append(path, cwd) == -1) {
        PyErr_Print();
        printf("Error: failed to append '.' to sys.path\n");
        status = -1;
        goto FAILED_APPEND; // 释放 cwd、path、sys
    }

    // 所有步骤成功，标记初始化完成
    is_initialized = 1;

    // 释放临时对象（局部引用，不影响解释器全局状态）
FAILED_APPEND:
    Py_DECREF(cwd); // 无论 append 是否成功，都要释放 cwd
FAILED_CWD:
    Py_DECREF(path); // 释放 sys.path 的局部引用
FAILED_PATH:
    Py_DECREF(sys);  // 释放 sys 模块的局部引用
FAILED_SYS:
    // 如果初始化失败，需要关闭未完全初始化的解释器
    if (status != 0 && Py_IsInitialized()) {
        Py_Finalize();
    }

    return status;
}


void face_final(void){

    Py_Finalize();

}


double face_category(void)
{
    double result = 0.0;
    system(WGET_CMD);

    if(0 == access(SEARCHFACE_FILE, F_OK))
    {
        return result;
    }


    if(face_init()==-1){
        
        printf("Error: face_init failed");
        return 0.0;
    }
    
    PyObject *pModule = PyImport_ImportModule("face");
    if(!pModule){
        PyErr_Print();
        printf("Error: failed to load face.py\n");
        goto FAILED_MODULE;
    }
    
    PyObject *pFunc = PyObject_GetAttrString(pModule, "alibaba_face");
    if(!pFunc){
        PyErr_Print();
        printf("Error: failed to load say_funny\n");
        goto  FAILED_FUNC;     
   }
    
    PyObject *pValue = PyObject_CallObject(pFunc, NULL);
    if(!pValue){
        PyErr_Print();
        printf("Error: function call failed\n");
        goto FAILED_VALUE; 
   }
    
    


    if(!PyArg_Parse(pValue, "d", &result)){
        PyErr_Print();
        printf("Error: parse failed");
        goto FAILED_RESULT; 
    }
        
    printf("result=%0.2lf\n", result);

FAILED_RESULT:
    Py_DECREF(pValue);
FAILED_VALUE:
    Py_DECREF(pFunc);
FAILED_FUNC:
    Py_DECREF(pModule);
FAILED_MODULE:



    return result;

}

/*
int main(int argc, char **argv)
{
    double face_result = 0.0;
    face_init();
    
    face_result = face_category();

    printf("face_result=%0.2lf\n", face_result);


    face_final();

    return 0;

}*/

