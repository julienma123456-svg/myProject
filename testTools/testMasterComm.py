# -*- coding: gbk -*-
import serial
import time

# ================= 基本参数 =================
HEADER = [0xA5, 0xA5]
ADDR   = 0xB1

TYPE_CMD  = 0x00
TYPE_DATA = 0x80

FUNC_CONTROL = 0x4D
FUNC_QUERY   = 0x5D
FUNC_DATA    = 0x6D
FUNC_RESET   = 0x7D

# ================= CRC8 =================
crc8_table = [
    0,94,188,226,97,63,221,131,194,156,126,32,163,253,31,65,
    157,195,33,127,252,162,64,30,95,1,227,189,62,96,130,220,
    35,125,159,193,66,28,254,160,225,191,93,3,128,222,60,98,
    190,224,2,92,223,129,99,61,124,34,192,158,29,67,161,255,
    70,24,250,164,39,121,155,197,132,218,56,102,229,187,89,7,
    219,133,103,57,186,228,6,88,25,71,165,251,120,38,196,154,
    101,59,217,135,4,90,184,230,167,249,27,69,198,152,122,36,
    248,166,68,26,153,199,37,123,58,100,134,216,91,5,231,185,
    140,210,48,110,237,179,81,15,78,16,242,172,47,113,147,205,
    17,79,173,243,112,46,204,146,211,141,111,49,178,236,14,80,
    175,241,19,77,206,144,114,44,109,51,209,143,12,82,176,238,
    50,108,142,208,83,13,239,177,240,174,76,18,145,207,45,115,
    202,148,118,40,171,245,23,73,8,86,180,234,105,55,213,139,
    87,9,235,181,54,104,138,212,149,203,41,119,244,170,72,22,
    233,183,85,11,136,214,52,106,43,117,151,201,74,20,246,168,
    116,42,200,150,21,75,169,247,182,232,10,84,215,137,107,53
]

def calc_crc(data):
    crc = 0
    for b in data:
        crc = crc8_table[crc ^ b]
    return crc


# ================= 帧构造 =================
def build_frame(func, payload, frame_type=TYPE_CMD):
    frame = []
    frame += HEADER
    frame.append(0x00)  # LEN占位
    frame.append(ADDR)
    frame.append(frame_type)
    frame.append(func)
    frame += payload

    frame[2] = len(frame) + 1  # +CRC
    crc = calc_crc(frame[2:])
    frame.append(crc)

    return bytes(frame)


# ================= 控制命令 =================
def build_control(onoff, power):
    """
    onoff: 0=开, 1=关
    power: float (如 23.4W)
    """

    gear = int(power // 10)
    level = int((power - gear * 10) * 10)

    # ? 必须填满长度=13
    payload = [
        onoff,      # pdta[6]
        gear,       # pdta[7]
        level       # pdta[8]
    ]

    # 填充到满足长度
    while len(payload) < (5):  # 总长13 - 固定头7 = payload长度6
        payload.append(0x00)
    payload.append(0x0c)
    return build_frame(FUNC_CONTROL, payload)


# ================= 查询 =================
def build_query():
    return build_frame(FUNC_QUERY, [])


# ================= 复位 =================
def build_reset():
    return build_frame(FUNC_RESET, [])


# ================= 数据解析 =================
def parse_frame(data):
    if len(data) < 7:
        return

    if data[0] != 0xA5 or data[1] != 0xA5:
        print("帧头错误")
        return

    length = data[2]
    crc_rx = data[length - 1]
    crc_calc = calc_crc(data[2:length-1])

    print("RAW:", data.hex())

    if crc_rx != crc_calc:
        print("CRC错误")
        return

    func = data[5]

    if func == FUNC_DATA:
        work = data[6]
        temp = data[7] - 25

        fwd = data[8]*10 + data[9]*0.1
        rev = data[10]*10 + data[11]*0.1

        rl_raw = (data[12]<<8) | data[13]
        if rl_raw & 0x8000:
            rl_raw = -((~rl_raw + 1) & 0xFFFF)
        rl = rl_raw / 10.0

        current = ((data[16]>>4) + (data[16]&0x0F)*0.1)

        print(f"""
===== DATA =====
状态: {work}
温度: {temp} ℃
正向功率: {fwd} W
反向功率: {rev} W
回波损耗: {rl} dB
电流: {current} A
""")

    else:
        print(f"FUNC: {hex(func)}")


# ================= 测试流程 =================
def main():
    ser = serial.Serial("COM5", 115200, timeout=1)

    # # ---- 查询 ----
    # frame = build_query()
    # print("发送 QUERY:", frame.hex())
    # ser.write(frame)
    # time.sleep(0.2)
    # parse_frame(ser.read(64))

    # ---- 控制 ----
    frame = build_control(onoff=1, power=120)
    print("发送 CONTROL:", frame.hex())
    ser.write(frame)
    time.sleep(0.2)
    parse_frame(ser.read(64))

    # # ---- 复位 ----
    # frame = build_reset()
    # print("发送 RESET:", frame.hex())
    # ser.write(frame)

    ser.close()


if __name__ == "__main__":
    main()