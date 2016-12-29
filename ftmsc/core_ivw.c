#include <Python.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "qivw.h"
#include "msp_cmn.h"
#include "msp_errors.h"

static PyObject *FtmscError;
static char ivw_msg[100];

static PyObject* pyQIVWInit(PyObject *self, PyObject *args)
{
    const char *init_str;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &init_str))
        return NULL;
	ret = MSPLogin(NULL, NULL, init_str);
    return Py_BuildValue("i", ret);
}

static PyObject* pyQIVWClose(PyObject *self, PyObject *args)
{
    int ret;
	ret = MSPLogout();
    return Py_BuildValue("i", ret);
}

static PyObject* pyQIVWGetMsg(PyObject *self, PyObject *args)
{
    return Py_BuildValue("s", ivw_msg);
}

int pyQIVWMsgProc( const char *sessionID, int msg, int param1, int param2, const void *info,void *userData )
{
    if (MSP_IVW_MSG_ERROR == msg) //唤醒出错消息
    {
        printf("\n\nMSP_IVW_MSG_ERROR errCode = %d\n\n", param1);
    }
    else if (MSP_IVW_MSG_WAKEUP == msg) //唤醒成功消息
    {
        //ivw_msg = (char*)info;
        strcpy(ivw_msg, info);
        //sprintf(ivw_msg, "%s", info);
    }
    return 0;
}

// static PyObject* pyQIVWMsgProc(PyObject *self, PyObject *args)
// {
//     int ret;
//     const char *sessid;
// 	ret = MSPLogout();
//     return Py_BuildValue("i", ret);
// }

static PyObject* pyQIVWRegisterNotify(PyObject *self, PyObject *args)
{
    const char *sessid;
    int ret;
    // PyObject *pyQIVWMsgProc;
    PyObject *userdata = NULL;

    if (!PyArg_ParseTuple(args, "s", &sessid)) 
        return NULL;
    ret = QIVWRegisterNotify(sessid, pyQIVWMsgProc, userdata);
    return Py_BuildValue("i", ret);
}

static PyObject* pyQIVWSessionBegin(PyObject *self, PyObject *args)
{
    const char *grammarList;
    const char *params;
    int err_code;

    const char* sessid;
    if (!PyArg_ParseTuple(args, "ss", &grammarList, &params))
        return NULL;
    
    sessid = QIVWSessionBegin(grammarList, params, &err_code);
    return Py_BuildValue("si", sessid, err_code);
}


static PyObject* pyQIVWAudioWrite(PyObject *self, PyObject *args)
{
    const char* sessid;
    const char* data;
    int dataLen;
    int audioStatus;

    if (!PyArg_ParseTuple(args, "sz#i", &sessid, &data, &dataLen, &audioStatus))
        return NULL;

    int ret;
    ret = QIVWAudioWrite(sessid, data, dataLen, audioStatus);
    return Py_BuildValue("i", ret);
}


static PyObject* pyQIVWSessionEnd(PyObject *self, PyObject *args)
{
    const char* sessid;
    const char* hints;

    int ret;
    if (!PyArg_ParseTuple(args, "ss", &sessid, &hints))
        return NULL;

    ret = QIVWSessionEnd(sessid, hints);
    return Py_BuildValue("i", ret);
}


static PyMethodDef FtmscMethods[] = {
    {"qivwInit", pyQIVWInit, METH_VARARGS, "exec QIVWInit"},
    {"qivwClose", pyQIVWClose, METH_VARARGS, "exec QIVWClose"},   
    {"qivwRegisterNotify", pyQIVWRegisterNotify, METH_VARARGS, "exec QIVWRegisterNotify"},  
    {"qivwSessionBegin", pyQIVWSessionBegin, METH_VARARGS, "exec QIVWSessionBegin"},
    {"qivwGetMsg", pyQIVWGetMsg, METH_VARARGS, "exec QIVWGetMsg"},
    {"qivwAudioWrite", pyQIVWAudioWrite, METH_VARARGS, "exec AudioWrite"},
    {"qivwSessionEnd", pyQIVWSessionEnd, METH_VARARGS, "exec SessionEnd"},
    {NULL, NULL, 0, NULL}  
};  

PyMODINIT_FUNC initcore(void)
{
    PyObject *m;
    m = Py_InitModule("ftmsc.core", FtmscMethods);
    if (m == NULL)
        return;
    FtmscError = PyErr_NewException("ftmsc.error", NULL, NULL);
    Py_INCREF(FtmscError);
    PyModule_AddObject(m, "error", FtmscError);
}

