import streamlit as st
import serial
import time
import plotly.express as px
import os
import operator

# streamlit run Posture_Detection/streamlit_app_no_bluetooth.py

def read_serial(port, data_queue):
    count = 5
    flag = 0
    value_dict = {}
    global_dict = {'Front Slouch':0, 'Back Slouch':0, 'Right Slouch':0, 'Left Slouch':0}
    placeholder = st.empty()
    ser = serial.Serial(port, 9600)
    while True:
        if ser.in_waiting > 0:
            flag+=1
            line = ser.readline().decode('utf-8').strip()
            data_queue.append(line)
            print(line)
            
            if flag%count ==0:
                maximum_key = max(value_dict.items(), key=operator.itemgetter(1))[0]
                print("Max:",maximum_key)
                global_dict[maximum_key]+=1
                display_val(placeholder, value_dict, global_dict)
                value_dict={}
                notify('Your posture is important! \nPlease sit upright :)', 'CorrectMyPosture')
                time.sleep(5)
                ser.read_all()
            else:
                if line[0]=='B':
                    value_dict['Back Slouch'] = float(line[-4:])
                elif line[0]=='F':
                    value_dict['Front Slouch'] = float(line[-4:])
                elif line[0]=='R':
                    value_dict['Right Slouch'] = float(line[-4:])
                elif line[0]=='L':
                    value_dict['Left Slouch'] = float(line[-4:])

def display_val(placeholder, value_dict, global_dict):
    # Display the dataframe in a scrollable window
    # Replace the chart with several elements:
    placeholder.empty()
    with placeholder.container():
        st.subheader('Serial Receive: \n') 
        # temp =""
        for i in value_dict:
            st.write(i +" : "+ str(value_dict[i]))
        st.subheader('Statistics: \n') 
        temp_fig = get_sorted_bar_plot(global_dict)
        st.plotly_chart(temp_fig, use_container_width=True)       

def notify(message, title):
    os.system(f"osascript -e 'display notification \"{message}\" with title \"{title}\"'")

def get_sorted_bar_plot(global_dict):
    maximum_key = max(global_dict.items(), key=operator.itemgetter(1))[0]
    colour_dict = {'Front Slouch':'teal', 'Back Slouch':'teal', 'Right Slouch':'teal', 'Left Slouch':'teal'}
    colour_dict[maximum_key]='blue'
    print(maximum_key)
    print(list(colour_dict.values()))

    global_dict = dict(sorted(global_dict.items()))
    colour_dict = dict(sorted(colour_dict.items()))

    fig = px.bar({'Gesture': list(global_dict.keys()), 'Label': list(global_dict.values())}, 'Gesture', 'Label', color=list(colour_dict.values()))
    return fig

st.set_page_config(page_title='CorrectMyPosture', page_icon=':person_in_lotus_position:')

st.title("CorrectMyPosture")

port = st.text_input('Enter serial port:', '/dev/cu.usbmodem14201')

print(port)

data_queue = []

serial_process = read_serial(port, data_queue)
