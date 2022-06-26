import math  

def make_vector(dot_a, dot_b):
    return (dot_b[0] - dot_a[0], dot_b[1] - dot_a[1], dot_b[2] - dot_a[2])

def adjust(res):
    return res

def analyze(landmarks, y_sum):
    throttle = 0
    row = 0
    pitch = 0

    vector_row = make_vector(landmarks[8], landmarks[20])
    vector_pitch = make_vector(landmarks[12], landmarks[9])

    y_average = y_sum / 21
    throttle = y_average  - 150
    if throttle <= 0:
        throttle = 0
    
    row = math.degrees(math.atan2(vector_row[1], vector_row[0]))
    pitch = (landmarks[9][1] - landmarks[12][1]) / 10 * 4 - 10

    row = row
    pitch = pitch

    return int(y_average), adjust(int(throttle)), adjust(int(row)), adjust(int(pitch))
