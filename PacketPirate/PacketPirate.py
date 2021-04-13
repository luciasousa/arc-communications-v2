from time import sleep

import gpxpy
from scapy.all import *
from threading import Thread, Lock
import sched
from geographiclib.geodesic import Geodesic
from math import radians, cos, sin, asin, sqrt




class CAM:

    def __init__(self, payload):
        self.message_id = int(payload[0:2], 16)
        self.station_id = int(payload[2:10], 16)
        self.generationtime = int(payload[10:18], 16)
        self.containermask = int(payload[18:20], 16)
        self.station_type = int(payload[20:28], 16)
        self.latitude = int(payload[28:36], 16)
        self.longitude = int(payload[36:44], 16)
        self.semiMajorAxisConfidence = int(payload[44:52], 16)
        self.semiMinorAxisConfidence = int(payload[52:60], 16)
        self.semiMajorOrientation = int(payload[60:68], 16)
        self.altitude = int(payload[68:76], 16)
        self.heading = int(payload[76:84], 16)
        self.headingConfidence = int(payload[84:92], 16)
        self.speed = int(payload[92:100], 16)
        self.speedConfidence = int(payload[100:108], 16)
        self.vehicleLength = int(payload[108:116], 16)
        self.vehicleWidth = int(payload[116:124], 16)
        self.longitudinalAcceleration = int(payload[124:132], 16)
        self.longitudinalAccelerationConfidence = int(payload[132:140], 16)
        self.yawRate = int(payload[140:148], 16)
        self.yawRateConfidence = int(payload[148:156], 16)
        self.vehicleRole = int(payload[156:164], 16)
        self.persons = int(payload[164:172], 16)
        self.airbags = int(payload[172:180], 16)
        self.abs = int(payload[180:182], 16)
        self.overturned = int(payload[182:184], 16)
        self.hazard_lights = int(payload[184:186], 16)
        self.all_seatbelts = int(payload[186:188], 16)
        self.accident = int(payload[188:190], 16)
        self.temperature = int(payload[190:198], 16)

    def dump_str(self):
        string = ""
        for attribute, value in self.__dict__.items():
            if attribute in ['abs', 'overturned', 'hazard_lights', 'all_seatbelts', 'accident','message_id',
                             'containermask']:
                string += format(value, '02x')
            else:
                if value < 0:
                    string += format(value & (2 ** 32 - 1), '08x')
                else:
                    string += format(value, '08x')

        return string

    def dump_fields(self):
        for attribute, value in self.__dict__.items():
            print(attribute + " = " + str(value))



scheduler = sched.scheduler(time.time, time.sleep)

def new_timed_call(calls_per_second, callback, *args, **kw):
    period = 1.0 / calls_per_second
    def reload():
        callback(*args, **kw)
        scheduler.enter(period, 0, reload, ())
    scheduler.enter(period, 0, reload, ())


mutex = Lock()
currentposition = [None, None, None]
previous_position = [None, None]


def calculate_initial_compass_bearing(pointA, pointB):

    if (type(pointA) != tuple) or (type(pointB) != tuple):
        raise TypeError("Only tuples are supported as arguments")

    lat1 = math.radians(pointA[0])
    lat2 = math.radians(pointB[0])

    diffLong = math.radians(pointB[1] - pointA[1])

    x = math.sin(diffLong) * math.cos(lat2)
    y = math.cos(lat1) * math.sin(lat2) - (math.sin(lat1)
            * math.cos(lat2) * math.cos(diffLong))

    initial_bearing = math.atan2(x, y)


    initial_bearing = math.degrees(initial_bearing)
    compass_bearing = (initial_bearing + 360) % 360

    return compass_bearing

def update_variables():
    gpx_file = open('route.gpx', 'r')
    gpx = gpxpy.parse(gpx_file)
    """ Convert 100km/hour in meter/second """
    speed = (100 * 1000) / 3600
    previous_point = None
    for track in gpx.tracks:
        for segment in track.segments:
            for i, point in enumerate(segment.points):
                if previous_point is not None:
                    #print(str(previous_point.longitude) + "  " + str(previous_point.latitude))
                    heading = (calculate_initial_compass_bearing(
                        (previous_point.latitude, previous_point.longitude),
                        (point.latitude, point.longitude)
                    ))
                    distance = point.distance_2d(previous_point)
                    duration = distance / speed
                    #print(str(distance) + "\n")
                    sleep(duration)
                    mutex.acquire()
                    try:
                        currentposition[0] = point.longitude
                        currentposition[1] = point.latitude
                        previous_position[0] = previous_point.longitude
                        previous_position[1] = previous_point.latitude
                    finally:
                        mutex.release()
                previous_point = point

def create_new(new_packet, vehicle_id=0, offset=0, accident_prob=0):
    temp = copy.deepcopy(new_packet)
    payload = temp["Raw"].load.hex()
    temp["UDP"].remove_payload()
    cam_message = CAM(payload)
    mutex.acquire()
    try:
        new_long = currentposition[0]
        new_lat = currentposition[1]
        prev_long = previous_position[0]
        prev_lat = previous_position[1]
    finally:
        mutex.release()
    if new_long is not None and new_lat is not None:
        if offset != 0:
            prev_lat = prev_lat + offset
            prev_long = prev_long + offset
            new_lat = new_lat + offset
            new_long = new_long + offset
        if vehicle_id != 0:
            cam_message.station_id = vehicle_id

        if accident_prob != 0:
            if random.random() < accident_prob:
                cam_message.accident = 1
                print("accident here")

        heading = (calculate_initial_compass_bearing(
            (prev_lat, prev_long),
            (new_lat, new_long)
        ))
        cam_message.longitude = int(new_long * pow(10,7))
        cam_message.latitude = int(new_lat * pow(10, 7))
        cam_message.heading = int(heading * pow(10, 7))
        new_payload = cam_message.dump_str()
        temp.add_payload(bytes.fromhex(new_payload))
        write(temp)



def write(packet):
    wrpcap('cenas.pcap', packet, append=True)


def main():
    threading.Thread(target=update_variables).start()
    packets = rdpcap("fake2.pcap")
    new_packet = packets[500]
    new_timed_call(10, create_new, new_packet, 100, 0.0001, 0)
    new_timed_call(10, create_new, new_packet, 101, 0, 0.005)
    new_timed_call(10, create_new, new_packet, 200, -0.0001, 0)
    scheduler.run()





if __name__ == '__main__':
    main()
