from ftmsc import qivw
import time
import json
appid = "5858d999"#use yourself appid
respath = 'fo|res/ivw/wakeupresource.jet'
workdir = '.'
def test1():
    conn = qivw.IftQIVW(appid, respath, workdir)
    conn.init()
    sess = conn.createSession(grammarList='', params="ivw_threshold=0:-20,sst=wakeup")
    print sess.sessid
    status = ''
    sess.register()
    with open('/root/Linux_awaken_1135_5858d999/x64/bin/audio/awake.pcm', 'rb') as fp:
        while not status:
            data = fp.read(6400)
            time.sleep(0.2)
            status = sess.detect(data)
            if status:
                print json.loads(status)
    del sess

if __name__ == '__main__':
    test1()

