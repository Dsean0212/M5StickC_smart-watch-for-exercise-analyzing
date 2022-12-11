#!/usr/bin/python

############################################################
# This code uses the Beebotte API, you must have an account.
# You can register here: http://beebotte.com/register
#############################################################

import time
import paho.mqtt.client as mqtt
import signal
import pandas as pd
import os

timeset = []

def handler(signum, frame):
    raise Exception("end of time")

def loop_forever():
    while 1:
       time.sleep(1)

# Will be called upon reception of CONNACK response from the server.
def on_connect(client, data, flags, rc):
    client.subscribe("Your_Channel/Your_resource", 1)
    client.subscribe("IoT_gogogo/gyro", 1)
    client.subscribe("IoT_gogogo/acc", 1)

def on_message(client, data, msg):
    print(msg.topic + " " + str(msg.payload))    
    
    nowtime = time.strftime("%Y-%m-%d %H:%M:%S",time.localtime())
    timeset.append(nowtime)

    with open('./test.txt','a+') as f:
         f.write(msg.topic + "," + msg.payload.decode("utf-8") + "\n")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Set the username to 'token:CHANNEL_TOKEN' before calling connect
client.username_pw_set("Your_Token")
# Alternatively, set the username to your SECRET KEY
#client.username_pw_set('YOUR_SECRET_KEY')
client.connect("mqtt.beebotte.com", 1883, 60)

client.loop_start()

signal.signal(signal.SIGALRM, handler)
signal.alarm(60) #timeout after 60s

try:
    loop_forever()
except Exception as exc:
    print(exc)

    ### data preprocessing ###

    df = pd.read_csv (r'./test.txt', header=None)

    timeOdd = timeset[1::2] # Elements from timeset starting from 1 iterating by 2
    timeEven = timeset[::2] # Elements from timeset starting from 0 iterating by 2

    if df.iloc[0, 0] == 'IoT_gogogo/gyro':
        df_gyro = df.iloc[lambda x: x.index % 2 == 0].reset_index(drop=True)
        df_acc = df.iloc[lambda x: x.index % 2 == 1].reset_index(drop=True)
        
        df_gyro = pd.DataFrame({'time':timeEven, 'gyroX':df_gyro.iloc[:, 1], 'gyroY':df_gyro.iloc[:, 2], 'gyroZ':df_gyro.iloc[:, 3]})
        df_acc = pd.DataFrame({'time':timeOdd, 'accX':df_acc.iloc[:, 1], 'accY':df_acc.iloc[:, 2], 'accZ':df_acc.iloc[:, 3]})

    else:
        df_gyro = df.iloc[lambda x: x.index % 2 == 1].reset_index(drop=True)
        df_acc = df.iloc[lambda x: x.index % 2 == 0].reset_index(drop=True)

        df_gyro = pd.DataFrame({'time':timeOdd, 'gyroX':df_gyro.iloc[:, 1], 'gyroY':df_gyro.iloc[:, 2], 'gyroZ':df_gyro.iloc[:, 3]})
        df_acc = pd.DataFrame({'time':timeEven, 'accX':df_acc.iloc[:, 1], 'accY':df_acc.iloc[:, 2], 'accZ':df_acc.iloc[:, 3]})

    df_gyro.gyroX = df_gyro.gyroX.str.replace("X:", "").astype(float)
    df_gyro.gyroY = df_gyro.gyroY.str.replace("Y:", "").astype(float)
    df_gyro.gyroZ = df_gyro.gyroZ.str.replace("Z:", "").astype(float)

    df_acc.accX = df_acc.accX.str.replace("X:", "").astype(float)
    df_acc.accY = df_acc.accY.str.replace("Y:", "").astype(float)
    df_acc.accZ = df_acc.accZ.str.replace("Z:", "").astype(float)

    print(df_gyro)
    print(df_acc)

    df_gyro.to_csv (r'./test_gyro.csv', index=None)
    df_acc.to_csv (r'./test_acc.csv', index=None)


    ### Initialize ###

    df_gyro = df_gyro.iloc[0:0] #clear the dataframe
    df_acc = df_acc.iloc[0:0]

    #open('./test.txt', 'w').close() # clear the .txt file
    os.remove('./test.txt')          # or remove the .txt file


