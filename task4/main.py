import os
import tkinter as tk
from tkinter import filedialog
import logging
from tkinter import messagebox

os.environ["TK_SILENCE_DEPRECATION"] = "1"  # 경고 메시지 숨김

logging.getLogger("scapy.runtime").setLevel(logging.ERROR)  # 경고 메시지 비활성화

from scapy.all import rdpcap, conf

def select_file():
    file_path = filedialog.askopenfilename(filetypes=[("PCAP Files", "*.pcap")])
    if file_path:
        file_label.config(text="선택한 파일: " + file_path)
        analyze_pcap(file_path)

def analyze_pcap(pcap_file):
    conf.ipv6_enabled = False  # IPv6 사용 비활성화

    packets = rdpcap(pcap_file) #pcap_file 읽어와 packets에 저장
    packet_count = len(packets) # 패킷 개수
    delay_times = []  # 패킷 지연시간 합계
    protocol_counts = {} # 패킷 개수
    lost_packets = 0 # 손실된 패킷 개수 
    prev_time = None

    for packet in packets:
        if 'IP' in packet and 'TCP' in packet:
            # 패킷이 IP와 TCP 계층인지 확인
            if prev_time is not None:
                delay_time = packet.time - prev_time
                delay_times.append(delay_time)
            prev_time = packet.time
            # 이전 패킷과의 시간 간격을 계산해 delay_times 리스트에 추가
            protocol = packet[1].name
            if protocol in protocol_counts:
                protocol_counts[protocol] += 1
            else:
                protocol_counts[protocol] = 1
            # 프로토콜 개수 세기
            if packet.haslayer('TCP') and packet['TCP'].flags & 0x4:
                lost_packets += 1
            # 패킷이 TCP 계층을 포함하며, RST 플래그가 설정된 경우, lost_packets 개수 증가
            # RST(reset) 플래그는, TCP연결을 초기화하거나, 비정상적인 상황이 발생해 연결 종료할 때 사용됨
    delay_avg = sum(delay_times) / len(delay_times) if delay_times else 0
    bandwidth_avg = sum(packet['IP'].len for packet in packets if 'IP' in packet) / packet_count if packet_count > 0 else 0
    # 각각 패킷 지연시간 평균, 대역폭의 평균 계산.
    result = "분석 결과:\n"
    if delay_avg > 1.0:
        result += "패킷 지연시간이 느리므로 병목 현상이 발생할 수 있습니다.\n"
    elif lost_packets > 10:
        result += "패킷 손실이 자주 발생하므로 네트워크 상태를 확인해야 합니다.\n"
    else:
        result += "네트워크 상태는 정상입니다.\n"
    # 네트워크 상태가 정상인지 파악하는 기준 2가지 : 패킷 지연시간, 패킷 손실 개수

    result += f"Packet Delay Average: {delay_avg} seconds\n"
    result += f"Bandwidth Average: {bandwidth_avg} bytes\n"
    result += "Protocols:\n"
    for protocol, count in protocol_counts.items():
        result += f"- Protocol {protocol}: {count} packets\n"
    result += f"Lost Packets: {lost_packets}"
    # 촤종적으로 각각 패킷 지연시간 평균, 대역폭의 평균, 프로토콜별 패킷 개수, 손실 패킷 개수 출력

    # 결과를 메시지 박스에 표시
    messagebox.showinfo("분석 결과", result)

# Tkinter 윈도우 생성
window = tk.Tk()

# 파일 선택 버튼 생성
select_button = tk.Button(window, text="파일 선택", command=select_file)
select_button.pack()

# 선택한 파일 표시 레이블 생성
file_label = tk.Label(window, text="선택한 파일: ")
file_label.pack()

# 윈도우 실행
window.mainloop()

