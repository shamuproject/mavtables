#!/bin/env python

from __future__ import (absolute_import, division,
                        print_function, unicode_literals)
from builtins import *

import os
import sys
from time import sleep
from argparse import ArgumentParser
from threading import Thread
from pymavlink import mavutil
from datetime import datetime, timedelta


def parse_args():
    parser = ArgumentParser(description='Send mavlink packets from file.')
    parser.add_argument('system', type=int, help='system ID')
    parser.add_argument('component', type=int, help='component ID')
    parser.add_argument('script', help='script file to run')
    parser.add_argument(
        '--mavlink1', action='store_true',
        help='force MAVLink v1.0 instead of v2.0')
    parser.add_argument(
        '--udp', action='store', help='UDP address:port to connect to')
    parser.add_argument(
        '--serial', action='store', help='serial port device string')
    args = vars(parser.parse_args())
    if not args['udp'] and not args['serial']:
        print('expected --udp or --serial option')
        sys.exit()
    if args['udp'] and args['serial']:
        print('expected --udp or --serial option, but not both')
        sys.exit()
    return args


def send_packet(mav, packet, system=0, component=0):
    def a(n):
        return tuple(range(0, n))
    def b(n):
        return bytes(range(0, n))
    if packet == 'SYS_STATUS': # 1
        mav.mav.sys_status_send(*a(13))
    elif packet == 'SYSTEM_TIME': # 2
        mav.mav.system_time_send(*a(2))
    elif packet == 'PING': # 4
        args = a(2) + (system, component)
        mav.mav.ping_send(*args)
    elif packet == 'CHANGE_OPERATOR_CONTROL': # 5
        args = (system,) + a(2) + (b(25),)
        mav.mav.change_operator_control_send(*args)
    elif packet == 'CHANGE_OPERATOR_CONTROL_ACK': # 6
        mav.mav.change_operator_control_ack_send(*a(3))
    elif packet == 'AUTH_KEY': # 7
        mav.mav.auth_key_send(b(32))
    elif packet == 'SET_MODE': # 11
        mav.mav.set_mode_send(system, *a(2))
    elif packet == 'PARAM_REQUEST_READ': # 20
        mav.mav.param_request_read_send(system, component, b(16), 1)
    elif packet == 'PARAM_REQUEST_LIST': # 21
        mav.mav.param_request_list_send(system, component)
    elif packet == 'PARAM_VALUE': # 22
        mav.mav.param_value_send(b(16), *a(4))
    elif packet == 'PARAM_SET': # 23
        mav.mav.param_set_send(system, component, b(16), *a(2))
    elif packet == 'GPS_RAW_INT': # 24
        if mav.mavlink20():
            mav.mav.gps_raw_int_send(*a(15))
        else:
            mav.mav.gps_raw_int_send(*a(10))
    elif packet == 'GPS_STATUS': # 25
        mav.mav.gps_status_send(1, b(20), b(20), b(20), b(20), b(20))
    elif packet == 'SCALED_IMU': # 26
        mav.mav.scaled_imu_send(*a(10))
    elif packet == 'RAW_IMU': # 27
        mav.mav.raw_imu_send(*a(10))
    elif packet == 'RAW_PRESSURE': # 28
        mav.mav.raw_pressure_send(*a(5))
    elif packet == 'SCALED_PRESSURE': # 29
        mav.mav.scaled_pressure_send(*a(4))
    elif packet == 'ATTITUDE': # 30
        mav.mav.attitude_send(*a(7))
    elif packet == 'ATTITUDE_QUATERNION': # 31
        mav.mav.attitude_quaternion_send(*a(8))
    elif packet == 'LOCAL_POSITION_NED': # 32
        mav.mav.local_position_ned_send(*a(7))
    elif packet == 'GLOBAL_POSITION_INT': # 33
        mav.mav.global_position_int_send(*a(9))
    elif packet == 'RC_CHANNELS_SCALED': # 34
        mav.mav.rc_channels_scaled_send(*a(11))
    elif packet == 'RC_CHANNELS_RAW': # 35
        mav.mav.rc_channels_raw_send(*a(11))
    elif packet == 'SERVO_OUTPUT_RAW': # 36
        if mav.mavlink20():
            mav.mav.servo_output_raw_send(*a(18))
        else:
            mav.mav.servo_output_raw_send(*a(10))
    elif packet == 'MISSION_REQUEST_PARTIAL_LIST': # 37
        if mav.mavlink20():
            mav.mav.mission_request_partial_list_send(system, component, *a(3))
        else:
            mav.mav.mission_request_partial_list_send(system, component, *a(2))
    elif packet == 'MISSION_WRITE_PARTIAL_LIST': # 38
        if mav.mavlink20():
            mav.mav.mission_write_partial_list_send(system, component, *a(3))
        else:
            mav.mav.mission_write_partial_list_send(system, component, *a(2))
    elif packet == 'MISSION_ITEM': # 39
        if mav.mavlink20():
            mav.mav.mission_item_send(system, component, *a(13))
        else:
            mav.mav.mission_item_send(system, component, *a(12))
    elif packet == 'MISSION_REQUEST': # 40
        if mav.mavlink20():
            mav.mav.mission_request_send(system, component, *a(2))
        else:
            mav.mav.mission_request_send(system, component, 1)
    elif packet == 'MISSION_SET_CURRENT': # 41
        mav.mav.mission_set_current_send(system, component, 1)
    elif packet == 'MISSION_CURRENT': # 42
        mav.mav.mission_current_send(1)
    elif packet == 'MISSION_REQUEST_LIST': # 43
        if mav.mavlink20():
            mav.mav.mission_request_list_send(system, component, 1)
        else:
            mav.mav.mission_request_list_send(system, component)
    elif packet == 'MISSION_COUNT': # 44
        if mav.mavlink20():
            mav.mav.mission_count_send(system, component, *a(2))
        else:
            mav.mav.mission_count_send(system, component, 1)
    elif packet == 'MISSION_CLEAR_ALL': # 45
        if mav.mavlink20():
            mav.mav.mission_clear_all_send(system, component, 1)
        else:
            mav.mav.mission_clear_all_send(system, component)
    elif packet == 'MISSION_ITEM_REACHED': # 46
        mav.mav.mission_item_reached_send(1)
    elif packet == 'MISSION_ACK': # 47
        if mav.mavlink20():
            mav.mav.mission_ack_send(system, component, *a(2))
        else:
            mav.mav.mission_ack_send(system, component, 1)
    elif packet == 'SET_GPS_GLOBAL_ORIGIN': # 48
        if mav.mavlink20():
            mav.mav.set_gps_global_origin_send(system, *a(4))
        else:
            mav.mav.set_gps_global_origin_send(system, *a(3))
    elif packet == 'GPS_GLOBAL_ORIGIN': # 49
        if mav.mavlink20():
            mav.mav.gps_global_origin_send(*a(4))
        else:
            mav.mav.gps_global_origin_send(*a(3))
    elif packet == 'PARAM_MAP_RC': # 50
        mav.mav.param_map_rc_send(system, component, b(16), *a(6))
    elif packet == 'MISSION_REQUEST_INT': # 51
        if mav.mavlink20():
            mav.mav.mission_request_int_send(system, component, *a(2))
        else:
            mav.mav.mission_request_int_send(system, component, 1)
    elif packet == 'SAFETY_SET_ALLOWED_AREA': # 54
        mav.mav.safety_set_allowed_area_send(system, component, *a(7))
    elif packet == 'SAFETY_ALLOWED_AREA': # 55
        mav.mav.safety_allowed_area_send(*a(7))
    elif packet == 'ATTITUDE_QUATERNION_COV': # 61
        args = (1, b(4)) + a(3) + (b(9),)
        mav.mav.attitude_quaternion_cov_send(*args)
    elif packet == 'NAV_CONTROLLER_OUTPUT': # 62
        mav.mav.nav_controller_output_send(*a(8))
    elif packet == 'GLOBAL_POSITION_INT_COV': # 63
        args = a(9) + (b(36),)
        mav.mav.global_position_int_cov_send(*args)
    elif packet == 'LOCAL_POSITION_NED_COV': # 64
        args = a(11) + (b(45),)
        mav.mav.local_position_ned_cov_send(*args)
    elif packet == 'RC_CHANNELS': # 65
        mav.mav.rc_channels_send(*a(21))
    elif packet == 'REQUEST_DATA_STREAM': # 66
        mav.mav.request_data_stream_send(system, component, *a(3))
    elif packet == 'DATA_STREAM': # 67
        mav.mav.data_stream_send(*a(3))
    elif packet == 'MANUAL_CONTROL': # 69
        mav.mav.manual_control_send(system, *a(5))
    elif packet == 'RC_CHANNELS_OVERRIDE': # 70
        mav.mav.rc_channels_override_send(system, component, *a(8))
    elif packet == 'MISSION_ITEM_INT': # 73
        if mav.mavlink20():
            mav.mav.mission_item_int_send(system, component, *a(13))
        else:
            mav.mav.mission_item_int_send(system, component, *a(12))
    elif packet == 'VFR_HUD': # 74
        mav.mav.vfr_hud_send(*a(6))
    elif packet == 'COMMAND_INT': # 75
        mav.mav.command_int_send(system, component, *a(11))
    elif packet == 'COMMAND_LONG': # 76
        mav.mav.command_long_send(system, component, *a(9))
    elif packet == 'COMMAND_ACK': # 77
        mav.mav.command_ack_send(*a(2))
    elif packet == 'MANUAL_SETPOINT': # 81
        mav.mav.manual_setpoint_send(*a(7))
    elif packet == 'SET_ATTITUDE_TARGET': # 82
        mav.mav.set_attitude_target_send(1, system, component, 1, b(4), *a(4))
    elif packet == 'ATTITUDE_TARGET': # 83
        args = a(2) + (b(4), ) + a(4)
        mav.mav.attitude_target_send(*args)
    elif packet == 'SET_POSITION_TARGET_LOCAL_NED': # 84
        mav.mav.set_position_target_local_ned_send(
            1, system, component, *a(13))
    elif packet == 'POSITION_TARGET_LOCAL_NED': # 85
        mav.mav.position_target_local_ned_send(*a(14))
    elif packet == 'SET_POSITION_TARGET_GLOBAL_INT': # 86
        mav.mav.set_position_target_global_int_send(
            1, system, component, *a(13))
    elif packet == 'POSITION_TARGET_GLOBAL_INT': # 87
        mav.mav.position_target_global_int_send(*a(14))
    elif packet == 'LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET': # 89
        mav.mav.local_position_ned_system_global_offset_send(*a(7))
    elif packet == 'HIL_STATE': # 90
        mav.mav.hil_state_send(*a(16))
    elif packet == 'HIL_CONTROLS': # 91
        mav.mav.hil_controls_send(*a(11))
    elif packet == 'HIL_RC_INPUTS_RAW': # 92
        mav.mav.hil_rc_inputs_raw_send(*a(14))
    elif packet == 'HIL_ACTUATOR_CONTROLS': # 93
        mav.mav.hil_actuator_controls_send(1, b(16), *a(2))
    elif packet == 'OPTICAL_FLOW': # 100
        if mav.mavlink20():
            mav.mav.optical_flow_send(*a(10))
        else:
            mav.mav.optical_flow_send(*a(8))
    elif packet == 'GLOBAL_VISION_POSITION_ESTIMATE': # 101
        if mav.mavlink20():
            args = a(7) + (b(21),)
            mav.mav.global_vision_position_estimate_send(*args)
        else:
            mav.mav.global_vision_position_estimate_send(*a(7))
    elif packet == 'VISION_POSITION_ESTIMATE': # 102
        if mav.mavlink20():
            args = a(7) + (b(21),)
            mav.mav.vision_position_estimate_send(*args)
        else:
            mav.mav.vision_position_estimate_send(*a(7))
    elif packet == 'VISION_SPEED_ESTIMATE': # 103
        if mav.mavlink20():
            args = a(4) + (b(9),)
            mav.mav.vision_speed_estimate_send(*args)
        else:
            mav.mav.vision_speed_estimate_send(*a(4))
    elif packet == 'VICON_POSITION_ESTIMATE': # 104
        if mav.mavlink20():
            args = a(7) + (b(21),)
            mav.mav.vicon_position_estimate_send(*args)
        else:
            mav.mav.vicon_position_estimate_send(*a(7))
    elif packet == 'HIGHRES_IMU': # 105
        mav.mav.highres_imu_send(*a(15))
    elif packet == 'OPTICAL_FLOW_RAD': # 106
        mav.mav.optical_flow_rad_send(*a(12))
    elif packet == 'HIL_SENSOR': # 107
        mav.mav.hil_sensor_send(*a(15))
    elif packet == 'SIM_STATE': # 108
        mav.mav.sim_state_send(*a(21))
    elif packet == 'RADIO_STATUS': # 109
        mav.mav.radio_status_send(*a(7))
    elif packet == 'FILE_TRANSFER_PROTOCOL': # 110
        mav.mav.file_transfer_protocol_send(0, system, component, b(251))
    elif packet == 'TIMESYNC': # 111
        mav.mav.timesync_send(*a(2))
    elif packet == 'CAMERA_TRIGGER': # 112
        mav.mav.camera_trigger_send(*a(2))
    elif packet == 'HIL_GPS': # 113
        mav.mav.hil_gps_send(*a(13))
    elif packet == 'HIL_OPTICAL_FLOW': # 114
        mav.mav.hil_optical_flow_send(*a(12))
    elif packet == 'HIL_STATE_QUATERNION': # 115
        mav.mav.hil_state_quaternion_send(1, b(4), *a(14))
    elif packet == 'SCALED_IMU2': # 116
        mav.mav.scaled_imu2_send(*a(10))
    elif packet == 'LOG_REQUEST_LIST': # 117
        mav.mav.log_request_list_send(system, component, *a(2))
    elif packet == 'LOG_ENTRY': # 118
        mav.mav.log_entry_send(*a(5))
    elif packet == 'LOG_REQUEST_DATA': # 119
        mav.mav.log_request_data_send(system, component, *a(3))
    elif packet == 'LOG_DATA': # 120
        args = a(3) + (b(90),)
        mav.mav.log_data_send(*args)
    elif packet == 'LOG_ERASE': # 121
        mav.mav.log_erase_send(system, component)
    elif packet == 'LOG_REQUEST_END': # 122
        mav.mav.log_request_end_send(system, component)
    elif packet == 'GPS_INJECT_DATA': # 123
        mav.mav.gps_inject_data_send(system, component, 110, b(110))
    elif packet == 'GPS2_RAW': # 124
        mav.mav.gps2_raw_send(*a(12))
    elif packet == 'POWER_STATUS': # 125
        mav.mav.power_status_send(*a(3))
    elif packet == 'SERIAL_CONTROL': # 126
        args = a(5) + (b(70),)
        mav.mav.serial_control_send(*args)
    elif packet == 'GPS_RTK': # 127
        mav.mav.gps_rtk_send(*a(13))
    elif packet == 'GPS2_RTK': # 128
        mav.mav.gps2_rtk_send(*a(13))
    elif packet == 'SCALED_IMU3': # 129
        mav.mav.scaled_imu3_send(*a(10))
    elif packet == 'DATA_TRANSMISSION_HANDSHAKE': # 130
        mav.mav.data_transmission_handshake_send(*a(7))
    elif packet == 'ENCAPSULATED_DATA': # 131
        mav.mav.encapsulated_data_send(1, b(253))
    elif packet == 'DISTANCE_SENSOR': # 132
        mav.mav.distance_sensor_send(*a(8))
    elif packet == 'TERRAIN_REQUEST': # 133
        mav.mav.terrain_request_send(*a(4))
    elif packet == 'TERRAIN_DATA': # 134
        args = a(4) + (b(16),)
        mav.mav.terrain_data_send(*args)
    elif packet == 'TERRAIN_CHECK': # 135
        mav.mav.terrain_check_send(*a(2))
    elif packet == 'TERRAIN_REPORT': # 136
        mav.mav.terrain_report_send(*a(7))
    elif packet == 'SCALED_PRESSURE2': # 137
        mav.mav.scaled_pressure2_send(*a(4))
    elif packet == 'ATT_POS_MOCAP': # 138
        if mav.mavlink20():
            args = (1, b(4)) + a(3) + (b(21),)
            mav.mav.att_pos_mocap_send(*args)
        else:
            mav.mav.att_pos_mocap_send(1, b(4), *a(3))
    elif packet == 'SET_ACTUATOR_CONTROL_TARGET': # 139
        args = a(2) + (system, component, b(8))
        mav.mav.set_actuator_control_target_send(*args)
    elif packet == 'ACTUATOR_CONTROL_TARGET': # 140
        args = a(2) + (b(8),)
        mav.mav.actuator_control_target_send(*args)
    elif packet == 'ALTITUDE': # 141
        mav.mav.altitude_send(*a(7))
    elif packet == 'RESOURCE_REQUEST': # 142
        args = a(2) + (b(120), 1, b(120))
        mav.mav.resource_request_send(*args)
    elif packet == 'SCALED_PRESSURE3': # 143
        mav.mav.scaled_pressure3_send(*a(4))
    elif packet == 'FOLLOW_TARGET': # 144
        args = a(5) + (b(3), b(3), b(4), b(3), b(3), 1)
        mav.mav.follow_target_send(*args)
    elif packet == 'CONTROL_SYSTEM_STATE': # 146
        args = a(11) + (b(3), b(3), b(4)) + a(3)
        mav.mav.control_system_state_send(*args)
    elif packet == 'BATTERY_STATUS': # 147
        args = a(4) + (b(10),) + a(4)
        mav.mav.battery_status_send(*args)
    elif packet == 'AUTOPILOT_VERSION': # 148
        if mav.mavlink20():
            args = a(5) + (b(8), b(8), b(8)) + a(3) + (b(18),)
            mav.mav.autopilot_version_send(*args)
        else:
            args = a(5) + (b(8), b(8), b(8)) + a(3)
            mav.mav.autopilot_version_send(*args)
    elif packet == 'LANDING_TARGET': # 149
        if mav.mavlink20():
            args = a(11) + (b(4),) + a(2)
            mav.mav.landing_target_send(*args)
        else:
            mav.mav.landing_target_send(*a(8))
    elif packet == 'ESTIMATOR_STATUS': # 230
        mav.mav.estimator_status_send(*a(10))
    elif packet == 'WIND_COV': # 231
        mav.mav.wind_cov_send(*a(9))
    elif packet == 'GPS_INPUT': # 232
        mav.mav.gps_input_send(*a(18))
    elif packet == 'GPS_RTCM_DATA': # 233
        args = a(2) + (b(180),)
        mav.mav.gps_rtcm_data_send(*args)
    elif packet == 'HIGH_LATENCY': # 234
        mav.mav.high_latency_send(*a(24))
    elif packet == 'VIBRATION': # 241
        mav.mav.vibration_send(*a(7))
    elif packet == 'HOME_POSITION': # 242
        if mav.mavlink20():
            args = a(6) + (b(4),) + a(4)
            mav.mav.home_position_send(*args)
        else:
            args = a(6) + (b(4),) + a(3)
            mav.mav.home_position_send(*args)
    elif packet == 'SET_HOME_POSITION': # 243
        if mav.mavlink20():
            args = (system,) + a(6) + (b(4),) + a(4)
            mav.mav.set_home_position_send(*args)
        else:
            args = (system,) + a(6) + (b(4),) + a(3)
            mav.mav.set_home_position_send(*args)
    elif packet == 'MESSAGE_INTERVAL': # 244
        mav.mav.message_interval_send(*a(2))
    elif packet == 'EXTENDED_SYS_STATE': # 245
        mav.mav.extended_sys_state_send(*a(2))
    elif packet == 'ADSB_VEHICLE': # 246
        args = a(8) + (b(9),) + a(4)
        mav.mav.adsb_vehicle_send(*args)
    elif packet == 'COLLISION': # 247
        mav.mav.collision_send(*a(7))
    elif packet == 'V2_EXTENSION': # 248
        mav.mav.v2_extension_send(1, system, component, 1, a(249))
    elif packet == 'MEMORY_VECT': # 249
        args = a(3) + (b(32),)
        mav.mav.memory_vect_send(*args)
    elif packet == 'DEBUG_VECT': # 250
        mav.mav.debug_vect_send(b(10), *a(4))
    elif packet == 'NAMED_VALUE_FLOAT': # 251
        mav.mav.named_value_float_send(1, b(10), 1)
    elif packet == 'NAMED_VALUE_INT': # 252
        mav.mav.named_value_int_send(1, b(10), 1)
    elif packet == 'STATUSTEXT': # 253
        mav.mav.statustext_send(1, b(50))
    elif packet == 'DEBUG': # 254
        mav.mav.debug_send(*a(3))
    elif packet == 'SETUP_SIGNING': # 256
        mav.mav.setup_signing_send(system, component, b(32), 1)
    elif packet == 'BUTTON_CHANGE': # 257
        mav.mav.button_change_send(*a(3))
    elif packet == 'PLAY_TUNE': # 258
        mav.mav.play_tune_send(system, component, b(30))
    elif packet == 'CAMERA_IMAGE_CAPTURED': # 263
        args = a(7) + (b(4),) + a(2) + (b(205),)
        mav.mav.camera_image_captured_send(*args)
    elif packet == 'FLIGHT_INFORMATION': # 264
        mav.mav.flight_information_send(*a(4))
    elif packet == 'MOUNT_ORIENTATION': # 265
        mav.mav.mount_orientation_send(*a(4))
    elif packet == 'LOGGING_DATA': # 266
        args = (system, component) + a(3) + (b(249),)
        mav.mav.logging_data_send(*args)
    elif packet == 'LOGGING_DATA_ACKED': # 267
        args = (system, component) + a(3) + (b(249),)
        mav.mav.logging_data_acked_send(*args)
    elif packet == 'LOGGING_ACK': # 268
        mav.mav.logging_ack_send(system, component, 1)
    elif packet == 'WIFI_CONFIG_AP': # 299
        mav.mav.wifi_config_ap_send(b(32), b(64))
    elif packet == 'PROTOCOL_VERSION': # 300
        args = a(3) + (b(8), b(8))
        mav.mav.protocol_version_send(*args)
    elif packet == 'UAVCAN_NODE_STATUS': # 310
        mav.mav.uavcan_node_status_send(*a(6))
    elif packet == 'UAVCAN_NODE_INFO': # 311
        args = a(2) + (b(80),) + a(2) + (b(16),) + a(3)
        mav.mav.uavcan_node_info_send(*args)
    elif packet == 'OBSTACLE_DISTANCE': # 330
        args = a(2) + (b(72),) + a(3)
        mav.mav.obstacle_distance_send(*args)
    else:
        print('unknown packet type {:s}'.format(packet))
        sys.exit(1)


def parse_line(line):
    parts = line.split(' to ')
    packet = parts[0]
    try:
        system, component = parts[1].split('.')
    except:
        system = 0
        component = 0
    return packet, int(system), int(component)


def parse_file(filename):
    with open(filename) as f:
        content = f.readlines()
    return [parse_line(x.strip()) for x in content]


def start_connection(args):
    if not args['mavlink1']:
        os.environ['MAVLINK20'] = '1'
    mavutil.set_dialect('common')
    if args['udp']:
        mav = mavutil.mavlink_connection('udpout:' + args['udp'],
            source_system=args['system'], source_component=args['component'])
    elif args['serial']:
        mav = mavutil.mavlink_connection(args['serial'],
            source_system=args['system'], source_component=args['component'])
    else:
        sys.exit()
    return mav


def main():
    args = parse_args()
    packets = parse_file(args['script'])
    mav = start_connection(args)
    last = datetime.now() - timedelta(seconds=180)
    for packet, system, component in packets:
        if ((datetime.now() - last) > timedelta(seconds=120)):
            last = datetime.now()
            mav.mav.heartbeat_send(0, 0, 0, 0, 0)
        send_packet(mav, packet, system, component)
        sleep(0.001)


if __name__ == '__main__':
    main()
