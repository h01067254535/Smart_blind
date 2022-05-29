import cv2
import mediapipe as mp
import numpy as np
import serial
import math
import time

def fdist(x1,y1,x2,y2):
    return math.sqrt(math.pow(x1 - x2,2)) + math.sqrt(math.pow(y1 - y2,2))

#인식할 손의 개수
max_num_hands = 1
create_idx = [0,0,0,0,0,0,0,0]
#인식할 제스처 key, value 
rps_gesture = {0:'stop', -1:'one', -2:'two', -3:'three', -4:'four', -5:'five', -6:'up', -7:'down', -8:'brightness control'}

before_data = 0
ser = serial.Serial('COM12',9600)
print("아두이노아 통신 시작")
# MediaPipe hands model

mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils

hands = mp_hands.Hands(
    max_num_hands=max_num_hands,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5)

# Gesture recognition model
file = np.genfromtxt('data/dd.csv', delimiter=',')
angle = file[:,:-1].astype(np.float32)
label = file[:, -1].astype(np.float32)

#openCV의 K-Nearest Neighbors 알고리즘 사용 
knn = cv2.ml.KNearest_create()
knn.train(angle, cv2.ml.ROW_SAMPLE, label)

# 웹캠을 열어준다. pc의 경우 0
cap = cv2.VideoCapture(1)


while cap.isOpened():
    # 웹캠이 열려있을 때 한 Frame씩 읽어옴 
    ret, img = cap.read()

    # 읽기 실패 -> 다음 반복
    if not ret:
        continue

    # 이미지 좌우 반전
    img = cv2.flip(img, 1)
    img = cv2.resize(img, dsize=(800,600))
    # openCV: BGR, Mediapipe: RGB 컬러 시스템 사용
    # openCV에서 읽어온 한 Frmame을 BGR -> RGB로 바꿈 
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    # Mediapipe 모델에 넣어주기 전, 전처리 작업
    result = hands.process(img)

    # 이미지 출력을 위해 다시 RGB -> BGR로 바꿈
    img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)

    #인식 된 손이 있따면, 
    if result.multi_hand_landmarks is not None:
        for res in result.multi_hand_landmarks:
            #0으로 가득찬 21x3 배열 생성 
            joint = np.zeros((21, 3))
            #joint배열에 x,y,z 좌표값 저장 = landmark의 x, y, z 저장 
            for j, lm in enumerate(res.landmark):
                joint[j] = [lm.x, lm.y, lm.z]

            # Compute angles between joints
            # 랜드마크의 좌표를 이용해 관절에 대한 벡터를 구해줌
            v1 = joint[[0,1,2,3,0,5,6,7,0,9,10,11,0,13,14,15,0,17,18,19],:] # Parent joint
            v2 = joint[[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20],:] # Child joint
            v = v2 - v1 # [20,3]

            # Normalize v
            # 단위 벡터를 구해줌
            v = v / np.linalg.norm(v, axis=1)[:, np.newaxis]


            # Get angle using arcos of dot product
            angle = np.arccos(np.einsum('nt,nt->n',
                v[[0,1,2,4,5,6,8,9,10,12,13,14,16,17,18],:], 
                v[[1,2,3,5,6,7,9,10,11,13,14,15,17,18,19],:])) # [15,]


            angle = np.degrees(angle) # Convert radian to degree

            # Inference gesture
            data = np.array([angle], dtype=np.float32)
            ret, results, neighbours, dist = knn.findNearest(data, 3)
            idx = int(results[0][0])
            if idx == -8 : 
                temp = fdist(res.landmark[4].x, res.landmark[4].y, res.landmark[8].x, res.landmark[8].y)

                dis = round(temp*500)
                if dis > 255:
                    dis = 256
                if dis < 5:
                    dis = 1
            

                cv2.putText(
                    img, text='Brightness: %d' % int((dis)), org=(10, 30),
                    fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1,
                    color=255, thickness=2)
                cv2.putText(img, text=rps_gesture[idx].upper(), org=(int(res.landmark[0].x * img.shape[1]), int(res.landmark[0].y * img.shape[0] + 20)), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1, color=(255, 255, 255), thickness=2)
                print((str(dis) + '\n').encode('utf-8'))
                ser.write((str(dis) + '\n').encode('utf-8'))
                

            else:
                create_idx[-idx]+=1 
                for i in range(len(create_idx)):
                    if create_idx[i]>=10:
                        idx=-i
                        create_idx = [0]*8
                        
                        if idx == -8 : 
                            temp = fdist(res.landmark[4].x, res.landmark[4].y, res.landmark[8].x, res.landmark[8].y)

                            dis = round(temp*250)
                            if dis > 255:
                                dis = 255
                            
                        

                            cv2.putText(
                                img, text='Brightness: %d' % int((dis)), org=(10, 30),
                                fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1,
                                color=255, thickness=2)
                            cv2.putText(img, text=rps_gesture[idx].upper(), org=(int(res.landmark[0].x * img.shape[1]), int(res.landmark[0].y * img.shape[0] + 20)), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1, color=(255, 255, 255), thickness=2)
                            ser.write((str(round(dis)) + '\n').encode('utf-8'))
                        

                        else:
                            if idx in rps_gesture.keys():
                                cv2.putText(img, text=rps_gesture[idx].upper(), org=(int(res.landmark[0].x * img.shape[1]), int(res.landmark[0].y * img.shape[0] + 20)), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1, color=(255, 255, 255), thickness=2)

                            
                                data = (str(idx) + '\n').encode('utf-8')
                                
                                
                                if before_data != idx: 
                                    before_data = idx
                                    print('데이터:{0}'.format(data))
                                    ser.write(data)
                        
                        before_data = idx 
            # Other gestures
            #cv2.putText(img, text=gesture[idx].upper(), org=(int(res.landmark[0].x * img.shape[1]), int(res.landmark[0].y * img.shape[0] + 20)), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1, color=(255, 255, 255), thickness=2)

            mp_drawing.draw_landmarks(img, res, mp_hands.HAND_CONNECTIONS)

    cv2.imshow('Game', img)
    if cv2.waitKey(1) == ord('q'):
        break