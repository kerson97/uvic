import struct
import sys
import datetime

numComplete = 0

class IP_Header:
    src_ip = None  # <type 'str'>
    dst_ip = None  # <type 'str'>
    ip_header_len = None  # <type 'int'>
    total_len = None  # <type 'int'>
    protocol = None

    def __init__(self):
        self.src_ip = None
        self.dst_ip = None
        self.ip_header_len = 0
        self.total_len = 0

    def ip_set(self, src_ip, dst_ip):
        self.src_ip = src_ip
        self.dst_ip = dst_ip

    def protocol_set(self, value):
        self.protocol = value

    def header_len_set(self, length):
        self.ip_header_len = length

    def total_len_set(self, length):
        self.total_len = length

    def get_IP(self, buffer1, buffer2):
        src_addr = struct.unpack('BBBB', buffer1)
        dst_addr = struct.unpack('BBBB', buffer2)
        s_ip = str(src_addr[0]) + '.' + str(src_addr[1]) + '.' + str(src_addr[2]) + '.' + str(src_addr[3])
        d_ip = str(dst_addr[0]) + '.' + str(dst_addr[1]) + '.' + str(dst_addr[2]) + '.' + str(dst_addr[3])
        self.ip_set(s_ip, d_ip)

    def get_header_len(self, value):
        result = struct.unpack('B', value)[0]
        length = (result & 15) * 4
        self.header_len_set(length)

    def get_total_len(self, buffer):
        num1 = ((buffer[0] & 240) >> 4) * 16 * 16 * 16
        num2 = (buffer[0] & 15) * 16 * 16
        num3 = ((buffer[1] & 240) >> 4) * 16
        num4 = (buffer[1] & 15)
        length = num1 + num2 + num3 + num4
        self.total_len_set(length)


class TCP_Header:
    src_port = 0
    dst_port = 0
    seq_num = 0
    ack_num = 0
    data_offset = 0
    flags = {}
    window_size = 0
    checksum = 0
    ugp = 0

    def __init__(self):
        self.src_port = 0
        self.dst_port = 0
        self.seq_num = 0
        self.ack_num = 0
        self.data_offset = 0
        self.flags = {}
        self.window_size = 0
        self.checksum = 0
        self.ugp = 0

    def src_port_set(self, src):
        self.src_port = src

    def dst_port_set(self, dst):
        self.dst_port = dst

    def seq_num_set(self, seq):
        self.seq_num = seq

    def ack_num_set(self, ack):
        self.ack_num = ack

    def data_offset_set(self, data_offset):
        self.data_offset = data_offset

    def flags_set(self, ack, rst, syn, fin):
        self.flags["ACK"] = ack
        self.flags["RST"] = rst
        self.flags["SYN"] = syn
        self.flags["FIN"] = fin

    def win_size_set(self, size):
        self.window_size = size

    def get_src_port(self, buffer):
        num1 = ((buffer[0] & 240) >> 4) * 16 * 16 * 16
        num2 = (buffer[0] & 15) * 16 * 16
        num3 = ((buffer[1] & 240) >> 4) * 16
        num4 = (buffer[1] & 15)
        port = num1 + num2 + num3 + num4
        self.src_port_set(port)
        # print(self.src_port)
        return None

    def get_dst_port(self, buffer):
        num1 = ((buffer[0] & 240) >> 4) * 16 * 16 * 16
        num2 = (buffer[0] & 15) * 16 * 16
        num3 = ((buffer[1] & 240) >> 4) * 16
        num4 = (buffer[1] & 15)
        port = num1 + num2 + num3 + num4
        self.dst_port_set(port)
        # print(self.dst_port)
        return None

    def get_seq_num(self, buffer):
        seq = struct.unpack(">I", buffer)[0]
        self.seq_num_set(seq)
        # print(seq)
        return None

    def get_ack_num(self, buffer):
        ack = struct.unpack('>I', buffer)[0]
        self.ack_num_set(ack)
        return None

    def get_flags(self, buffer):
        value = struct.unpack("B", buffer)[0]
        fin = value & 1
        syn = (value & 2) >> 1
        rst = (value & 4) >> 2
        ack = (value & 16) >> 4
        self.flags_set(ack, rst, syn, fin)
        return None

    def get_window_size(self, buffer1, buffer2):
        buffer = buffer2 + buffer1
        size = struct.unpack('H', buffer)[0]
        self.win_size_set(size)
        return None

    def get_data_offset(self, buffer):
        value = struct.unpack("B", buffer)[0]
        length = ((value & 240) >> 4) * 4
        self.data_offset_set(length)
        # print(self.data_offset)
        return None

    def relative_seq_num(self, orig_num):
        if (self.seq_num >= orig_num):
            relative_seq = self.seq_num - orig_num
            self.seq_num_set(relative_seq)
        # print(self.seq_num)

    def relative_ack_num(self, orig_num):
        if (self.ack_num >= orig_num):
            relative_ack = self.ack_num - orig_num + 1
            self.ack_num_set(relative_ack)


class packet():
    # pcap_hd_info = None
    IP_header = None
    TCP_header = None
    timestamp = 0
    packet_No = 0
    RTT_value = 0
    size = 0
    RTT_flag = False
    buffer = None
    windowSize = 0

    def __init__(self):
        self.IP_header = IP_Header()
        self.TCP_header = TCP_Header()
        # self.pcap_hd_info = pcap_ph_info()
        self.timestamp = 0
        self.packet_No = 0
        self.RTT_value = 0.0
        self.RTT_flag = False
        self.buffer = None
        self.windowSize = 0

    def timestamp_set(self, buffer1, buffer2, orig_time):
        seconds = struct.unpack('I', buffer1)[0]
        microseconds = struct.unpack('<I', buffer2)[0]
        self.timestamp = round(seconds + microseconds * 0.000001 - orig_time, 6)
        # print(self.timestamp,self.packet_No)

    def size_set(self, size):
        self.size = size

    def packet_No_set(self, number):
        self.packet_No = number
        # print(self.packet_No)

    def get_RTT_value(self, p):
        rtt = p.timestamp - self.timestamp
        self.RTT_value = round(rtt, 8)


class TCP_Connection:
    connectionNumber = None
    totalPackets = 0
    SOURCE_IP = None
    SOURCE_PORT = None
    DEST_IP = None
    DEST_PORT = None
    num_packets = None
    isComplete = False
    startTime = None
    startEpoch = None
    endEpoch = None
    endTime = None
    duration = None
    synCount = 0
    finCount = 0
    reset = 0
    senderpackets = None
    receiverpackets = None
    sender = None
    status = "N/A"

    minSenderWindow = 0
    maxSenderWindow = 0
    avgWindowSize = 0

    bytesSent = 0
    bytesReceived = 0
    totalBytes = 0

    def __init__(self):
        self.SOURCE_IP = None
        self.SOURCE_PORT = None
        self.DEST_IP = None
        self.DEST_PORT = None
        self.connectionNumber = None
        self.num_packets = None
        self.isComplete = False
        self.startTime = None
        self.endTime = None
        self.duration = None
        self.startEpoch = 0
        self.endEpoch = 0
        self.senderpackets = 0
        self.receiverpackets = 0
        self.synCount = 0
        self.finCount = 0
        self.status = "N/A"
        self.totalPackets = 0
        self.sender = None
        self.minSenderWindow = 0
        self.maxSenderWindow = 0
        self.avgWindowSize = 0
        self.bytesSent = 0
        self.bytesReceived = 0
        self.totalBytes = 0

    def ip_set(self, src_ip, dst_ip):
        self.SOURCE_IP = src_ip
        self.DEST_IP = dst_ip

    def calcTotalBytes(self):
        self.totalBytes = self.bytesSent + self.bytesReceived

    def setAvgWindow(self):
        self.avgWindowSize = self.avgWindowSize / self.totalPackets

    def port_set(self, src_prt, dst_prt):
        self.SOURCE_PORT = src_prt
        self.DEST_PORT = dst_prt

    def conn_set(self, val):
        self.connectionNumber = val

    def pnum_set(self, val):
        self.num_packets = val

    def completed(self, val): \
            self.isComplete = True

    def startTimeSet(self, epock):
        self.startTime = datetime.datetime.fromtimestamp(epock).strftime('%M:%S')

    def endTimeSet(self, epock):
        self.endTime = datetime.datetime.fromtimestamp(epock).strftime('%M:%S')

    def durationSet(self, start, end):
        duration = start - end
        self.duration = datetime.datetime.fromtimestamp(duration).strftime('%M:%S:%f')

    def statusSet(self):
        global numComplete
        self.status = "S" + str(self.synCount) + "F" + str(self.finCount)
        if self.reset >= 1:
            self.status = "S" + str(self.synCount) + "F" + str(self.finCount) + "/R"
        if self.synCount and self.finCount >= 1:
            self.isComplete = True
            numComplete += 1

    def printConnection(self):
        print("Source IP: ", self.SOURCE_IP, ' | ', "Dest IP: ", self.DEST_IP, ' | ', "Source Port: ", self.SOURCE_PORT,
              ' | ', "Dest Port: ", self.DEST_PORT)


def sameConnection(connection1, connection2):
    # if theyre the same

    if (connection1.SOURCE_IP == connection2.SOURCE_IP and
            connection1.SOURCE_PORT == connection2.SOURCE_PORT and
            connection1.DEST_IP == connection2.DEST_IP and
            connection1.DEST_PORT == connection2.DEST_PORT):
        return 1

    # if theyre opposite
    if (connection1.SOURCE_IP == connection2.DEST_IP and
            connection1.SOURCE_PORT == connection2.DEST_PORT):
        return 1

    else:
        return 0


def packetScan(connections):
    # packet info
    ipHeader = IP_Header()
    TCPHeader = TCP_Header()
    newPacket = packet()
    connectionPackets = 0
    try:
        newPacket.timestamp_set(file.read(4), file.read(4), 0)  # set timestamp & packet num
        newPacket.packet_No_set(connectionPackets)
        newPacket.size_set(int.from_bytes(file.read(4), byteorder=sys.byteorder))
        file.read(18)  # skip incl_len and orig_len, skip ethernet header
        # IP header info
        ipHeader.get_header_len(file.read(1))
        file.read(1)  # more skip
        ipHeader.get_total_len(file.read(2))
        file.read(5)  # skip more
        ipHeader.protocol_set(file.read(1))
        file.read(2)  # skip
        ipSRC = file.read(4)
        ipDST = file.read(4)
        ipHeader.get_IP(ipSRC, ipDST)
        newPacket.IP_header = ipHeader  # set packets IP header to this one

        # TCP header info

        TCPHeader.get_src_port(file.read(2))
        TCPHeader.get_dst_port(file.read(2))

        TCPHeader.get_seq_num(file.read(4))
        TCPHeader.get_ack_num(file.read(4))
        TCPHeader.get_data_offset(file.read(1))
        TCPHeader.get_flags(file.read(1))
        TCPHeader.get_window_size(file.read(1), file.read(1))

        sizeInBytes = ipHeader.total_len - (ipHeader.ip_header_len + TCPHeader.data_offset)

        # create dummy connections for this packet

        dummy = TCP_Connection()
        dummy.ip_set(newPacket.IP_header.src_ip, newPacket.IP_header.dst_ip)
        dummy.port_set(TCPHeader.src_port, TCPHeader.dst_port)
        dummy.startTimeSet(newPacket.timestamp)
        dummy.startEpoch = newPacket.timestamp

        # if list length is 0 add dummy

        if len(connections) is 0:
            dummy.startTimeSet(newPacket.timestamp)
            dummy.startEpoch = newPacket.timestamp
            dummy.minSenderWindow = TCPHeader.window_size
            dummy.maxSenderWindow = TCPHeader.window_size
            dummy.bytesSent += sizeInBytes
            connections.append(dummy)

        # check connection list, update or create new connection

        for connection in connections:
            if sameConnection(connection, dummy) == 1:
                # update flags, packets sent, end time etc
                if TCPHeader.flags.get("FIN"):
                    connection.finCount += 1
                if TCPHeader.flags.get("SYN"):
                    connection.synCount += 1
                if TCPHeader.flags.get("RST"):
                    connection.reset += 1
                if connection.finCount >= 1 and connection.synCount >= 1:
                    connection.isComplete = True
                connection.statusSet()

                if connection.SOURCE_IP == dummy.SOURCE_IP:
                    connection.senderpackets += 1
                    connection.totalPackets += 1
                    connection.bytesSent += sizeInBytes
                else:
                    connection.receiverpackets += 1
                    connection.totalPackets += 1
                    connection.bytesReceived += sizeInBytes

                connection.endTimeSet(newPacket.timestamp)
                connection.endEpoch = newPacket.timestamp
                connection.durationSet(newPacket.timestamp, connection.startEpoch)

                if connection.minSenderWindow > TCPHeader.window_size:
                    connection.minSenderWindow = TCPHeader.window_size
                if connection.maxSenderWindow < TCPHeader.window_size:
                    connection.maxSenderWindow = TCPHeader.window_size

                connection.avgWindowSize += TCPHeader.window_size

                file.read(newPacket.size - 50)
                return connections

        # append dummy connection if match wasn't found
        if TCPHeader.flags.get("FIN"):
            dummy.finCount += 1
        if TCPHeader.flags.get("SYN"):
            dummy.synCount += 1
        if TCPHeader.flags.get("RST"):
            dummy.reset += 1
        dummy.senderpackets += 1
        dummy.totalPackets += 1
        dummy.bytesSent += sizeInBytes
        dummy.avgWindowSize += TCPHeader.window_size
        dummy.statusSet()
        connections.append(dummy)

        file.read(newPacket.size - 50)

    except:
        return connections


def main():
    global file

    file = open(sys.argv[1], "rb")
    file.read(24)  # skip global header
    connections = []
    numComplete = 0
    numReset = 0

    i = 0
    while i < 1549:
        packetScan(connections)
        i += 1

    meanTime = datetime.timedelta(seconds=0)
    maxTime = connections[0].duration
    minTime = connections[0].duration
    minPackets = connections[0].totalPackets
    meanPackets = 0
    maxPackets = connections[0].totalPackets

    maxWindowSize = connections[0].maxSenderWindow
    minWindowSize = connections[0].minSenderWindow
    avgWindowSize = 0

    i = 1
    for connection in connections:
        connection.calcTotalBytes()
        if connection.isComplete:
            numComplete += 1
        print("Connection: ", i)
        print("Source IP: ", connection.SOURCE_IP)
        print("Destination IP: ", connection.DEST_IP)
        print("Source Port: ", connection.SOURCE_PORT)
        print("Destination Port: ", connection.DEST_PORT)
        print("Status: ", connection.status)
        if "R" in connection.status:
            numReset += 1
        if connection.isComplete:
            print("Start Time: ", connection.startTime)
            print("End Time: ", connection.endTime)
            print("Duration: ", connection.duration)
            print("Source to Destination (packets): ", connection.senderpackets)
            print("Destination to Source (packets): ", connection.receiverpackets)
            print("Total Packets: ", connection.senderpackets + connection.receiverpackets)
            print("Data from source to destination (bytes): ", connection.bytesSent)
            print("Data from destination to source (bytes): ", connection.bytesReceived)
            print("Total bytes sent: ", connection.totalBytes, "\n")

            meanPackets += connection.totalPackets
            if connection.totalPackets > maxPackets:
                maxPackets = connection.totalPackets
            if connection.totalPackets < minPackets:
                minPackets = connection.totalPackets

            if connection.duration > maxTime:
                maxTime = connection.duration
            if connection.duration < minTime:
                minTime = connection.duration

            if connection.minSenderWindow < minWindowSize:
                minWindowSize = connection.minSenderWindow

            if connection.maxSenderWindow > maxWindowSize:
                maxWindowSize = connection.maxSenderWindow

            connection.setAvgWindow()
            avgWindowSize += connection.avgWindowSize

        print("\n")
        i += 1

    print("----------------------------------------------------------------------------------")
    print("General Information: ", "\n")
    print("Total Connections: ", len(connections))
    print("Complete TCP Connections: ", numComplete)
    print("Number of reset TCP connections: ", numReset)
    print("Number of TCP connections still open: ", i - numComplete - 1)
    print("----------------------------------------------------------------------------------", "\n")
    print("Complete TCP Connections: ", "\n")

    print("Minimum time duration: ", minTime, "seconds")
    print("Mean time duration: ", meanTime, "seconds")
    print("Maximum time duration: ", maxTime, "seconds", "\n")

    print("Minimum number of sent/received packets: ", minPackets)
    print("Mean number of sent/received packets: ", meanPackets / numComplete)
    print("Maximum number of sent/received packets: ", maxPackets, "\n")

    print("Minimum window size: ", minWindowSize, "bytes")
    avgWindowSize = avgWindowSize / numComplete
    print("Mean window size: ", avgWindowSize, "bytes")
    print("Maximum window size: ", maxWindowSize, "bytes")


if __name__ == "__main__":
    main()
