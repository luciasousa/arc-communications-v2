from scapy.all import *


pkts = rdpcap("fake.pcap")

pkts[400].show()
