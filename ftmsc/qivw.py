from ftmsc import core
import time

__all__ = ['IftQIVW', 'QIVWSession']


class IftQIVW(object):
    def __init__(self, appid = None, respath = 'fo|res/ivw/wakeupresource.jet', workdir = '.', timeout = None):
        self.appid = appid
        self.respath = respath
        self.workdir = workdir
        self.timeout = timeout
        self.init_flag = False

    def init(self):
        if not self.appid:
            print 'appid can\'t be None'
            return
        init_str = 'appid = %s,engine_start = ivw,ivw_res_path =%s, work_dir = %s' % (self.appid, self.respath, self.workdir)

        print 'qivw init str is %s'%init_str
        err = core.qivwInit(init_str)
        if err != 0:
            print 'qisr init error, error no is %s'%err
        else:
            self.init_flag = True
        return err

    def createSession(self,lazy=False,*args, **kwargs):
        if not self.init_flag:
            print 'init qivw first'
            return
        if 'grammarList' not in kwargs and 'params' not in kwargs:
            raise QIVWSessionParamException("require parameter 'grammarList' and 'params' when you init QISRSession")
        return QIVWSession(kwargs['grammarList'], kwargs['params'], lazy)

    def fini(self):
        err = core.qivwClose()
        self.init_flag = False
        if err != 0:
            print 'qisr fini error, error no is %s'%err


class QIVWSessionParamException(BaseException): pass


class QIVWSession(object):
    def __init__(self, grammarList, params,lazy=False):
        self.sessid = None
        self.grammarList = grammarList
        self.params = params
        self.getResult_flag = False
        if not lazy:
            self.begin()

    def __del__(self):
        if self.sessid:
            err = core.qivwSessionEnd(self.sessid, "normal end")
            if err != 0:
                print "session end error, sessid is %s and error no is %s"%(self.sessid, err)

    def begin(self):
        if self.sessid:
            return
        sessid, err = core.qivwSessionBegin(self.grammarList, self.params)
        if err != 0:
            raise QIVWSessionParamException('wakeup session begin error, error no is %s'%err)

        self.sessid = sessid
        print 'wakeup session begin success'

    def register(self):
        core.qivwRegisterNotify(self.sessid)

    def detect(self, data):
        status = ''
        audio_stat = 2
        err = core.qivwAudioWrite(self.sessid, data, audio_stat)
        status = core.qivwGetMsg()
        if err != 0:
            raise QIVWSessionParamException('wakeup detect, error no is %s'%err)
        return status

    def uploadAudio(self, fileObj):
        if not hasattr(fileObj, 'read'):
            raise QIVWSessionParamException("uploadAudio function except file like object")
        data = fileObj.read(6400)
        core.qivwRegisterNotify(self.sessid)
        while len(data) == 6400 :
            audio_stat = 2
            if len(data) < 6400:
                audio_stat = 4
            err = core.qivwAudioWrite(self.sessid, data, audio_stat)
            status = core.qivwGetMsg()
            print err, status
            if err != 0:
                raise QIVWSessionParamException('qisr upload audio error, error no is %s'%err)
            time.sleep(0.2)
            data = fileObj.read(6400)

        err = core.qivwAudioWrite(self.sessid, data, 4)
        if err != 0:
            raise QIVWSessionParamException('qisr upload audio error, error no is %s'%err)

