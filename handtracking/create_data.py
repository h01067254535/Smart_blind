import cv2
import mediapipe as mp
import numpy as np
import time, os
import pandas as pd
import openpyxl

n = input("학습시킬 라벨: ")
anglelist = []
seq_length = 30
secs_for_action = 10

# MediaPipe hands model
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
hands = mp_hands.Hands(
    max_num_hands=1,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5)

cap = cv2.VideoCapture(0)

created_time = int(time.time())

while cap.isOpened():
    
    data = []

    ret, img = cap.read()

    img = cv2.flip(img, 1)

    print('대기시간 3초')
    cv2.waitKey(3000)

    start_time = time.time()
    
    while time.time() - start_time < secs_for_action:
        ret, img = cap.read()

        img = cv2.flip(img, 1)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        result = hands.process(img)
        img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)

        if result.multi_hand_landmarks is not None:
            for res in result.multi_hand_landmarks:
                joint = np.zeros((21, 4))
                for j, lm in enumerate(res.landmark):
                    joint[j] = [lm.x, lm.y, lm.z, lm.visibility]

                # Compute angles between joints
                v1 = joint[[0,1,2,3,0,5,6,7,0,9,10,11,0,13,14,15,0,17,18,19], :3] # Parent joint
                v2 = joint[[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20], :3] # Child joint
                v = v2 - v1 # [20, 3]
                # Normalize v
                v = v / np.linalg.norm(v, axis=1)[:, np.newaxis]

                # Get angle using arcos of dot product
                angle = np.arccos(np.einsum('nt,nt->n',
                    v[[0,1,2,4,5,6,8,9,10,12,13,14,16,17,18],:], 
                    v[[1,2,3,5,6,7,9,10,11,13,14,15,17,18,19],:])) # [15,]

                
                angle = np.degrees(angle) # Convert radian to degree
                angle = np.append(angle, np.array(n))
                anglelist.append(angle)
                df = pd.DataFrame(anglelist)
                print(df)
                path = 'C:/Users/hanbonghak/Documents/GitHub/Smart_blind/handtracking/data/dd.csv'
                df.to_csv(path,mode='a',header=False,index=False)
                
                

                mp_drawing.draw_landmarks(img, res, mp_hands.HAND_CONNECTIONS)

        cv2.imshow('img', img)
        if cv2.waitKey(1) == ord('q'):
            break

    break