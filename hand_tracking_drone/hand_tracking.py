import cv2
import mediapipe as mp
import time
import serial
import landmark_analyze

cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
mpHands = mp.solutions.hands
mpHands = mp.solutions.mediapipe.python.solutions.hands
hands = mpHands.Hands(max_num_hands=1)
mpDraw = mp.solutions.drawing_utils

pTime = 0
cTime = 0

cnt = 0

arduino = serial.Serial(port='COM7', baudrate=9600)

while True:
    ret, img = cap.read()
    if ret:
        img = cv2.flip(img, 1)
        landmarks = [[0, 0, 0] for i in range(22)]
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        result = hands.process(imgRGB)

        imgHeight = img.shape[0]
        imgWidth = img.shape[1]

        y_sum = 0
        y_average = 0

        throttle = 0
        row = 0
        pitch = 0
        yaw = 0

        if result.multi_hand_landmarks:
            for handLms in result.multi_hand_landmarks:
                mpDraw.draw_landmarks(img, handLms, mpHands.HAND_CONNECTIONS)
                for i, lm in enumerate(handLms.landmark):
                    xPos = int(lm.x*imgWidth)
                    yPos = int(lm.y*imgHeight)
                    zPos = int(lm.z*100)
                    cv2.putText(img, str(i), (xPos-25, yPos-5), cv2.FONT_HERSHEY_COMPLEX, 0.4, (0, 0, 255), 2)
                    #print(i, xPos, yPos, zPos)
                    y_sum = y_sum + (imgHeight - yPos)
                    landmarks[i] = [xPos, imgHeight - yPos, zPos]

        cTime = time.time()
        fps = 1/(cTime - pTime)
        pTime = cTime

        y_average, throttle, row, pitch = landmark_analyze.analyze(landmarks, y_sum)
        
        
        row = row + 90
        pitch = pitch + 90
        

        cnt = cnt + 1
        if cnt == 1:
            cnt = 0
            arduino.write(bytes(str(throttle), 'utf-8'))
            arduino.write(bytes(str(','), 'utf-8'))
            arduino.write(bytes(str(row), 'utf-8'))
            arduino.write(bytes(str(','), 'utf-8'))
            arduino.write(bytes(str(pitch), 'utf-8'))
            arduino.write(bytes(str('\n'), 'utf-8'))

        res = [0, 0, 0, 0, 0, 0, 0]
        for i in range(7): 
            res[i] = arduino.readline().decode('utf-8')
            

        print(res)

        row = row - 90
        pitch = pitch - 90

        cv2.putText(img, f"FPS : {int(fps)}", (30, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (255, 0, 0), 3)
        cv2.putText(img, f"throttle : {int(throttle)}", (30, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (255, 0, 0), 3)
        cv2.putText(img, f"row : {int(row)}", (30, 120), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (255, 0, 0), 3)
        cv2.putText(img, f"pitch : {int(pitch)}", (30, 160), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (255, 0, 0), 3)
        cv2.putText(img, f"y_average : {int(y_average)}", (30, 200), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (255, 0, 0), 3)
        
        cv2.line(img, (landmarks[8][0], imgHeight - landmarks[8][1]), (landmarks[20][0], imgHeight - landmarks[20][1]), (255, 0, 0), 5)
        cv2.line(img, (landmarks[12][0], imgHeight - landmarks[12][1]), (landmarks[9][0], imgHeight - landmarks[9][1]), (0, 255, 0), 5)

        cv2.imshow('img', img)

    if cv2.waitKey(1) == ord('q'):
        break