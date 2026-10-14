import 'dart:convert';

import 'package:portachiave_mobile/core/network/connection_state.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class DeviceModel {
  final String id;
  final String name;
  final String firmware;
  final bool isConnected;
  final double freeMemory;
  final int uptime;
  final bool isAuthenticated;
  final ConnectionState connectionState;

  DeviceModel({
    required this.id,
    required this.name,
    required this.firmware,
    required this.isConnected,
    required this.freeMemory,
    required this.uptime,
    required this.isAuthenticated,
    required this.connectionState,
  });

  factory DeviceModel.fromJson(Map<String, dynamic> json) => DeviceModel(
        id: json['id'] ?? '',
        name: json['name'] ?? 'PortaChiave',
        firmware: json['firmware'] ?? '1.0.0',
        isConnected: json['isConnected'] ?? false,
        freeMemory: (json['freeMemory'] ?? 0).toDouble(),
        uptime: json['uptime'] ?? 0,
        isAuthenticated: json['isAuthenticated'] ?? false,
        connectionState: _parseConnectionState(json['connectionState'] ?? 'disconnected'),
      );

  static ConnectionState _parseConnectionState(String state) {
    switch (state.toLowerCase()) {
      case 'connected':
        return ConnectionState.connected;
      case 'connecting':
        return ConnectionState.connecting;
      case 'disconnected':
        return ConnectionState.disconnected;
      case 'timeout':
        return ConnectionState.timeout;
      case 'apierror':
      case 'api_error':
        return ConnectionState.apiError;
      case 'firmwareincompatible':
      case 'firmware_incompatible':
        return ConnectionState.firmwareIncompatible;
      case 'networkerror':
      case 'network_error':
        return ConnectionState.networkError;
      default:
        return ConnectionState.disconnected;
    }
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'name': name,
        'firmware': firmware,
        'isConnected': isConnected,
        'freeMemory': freeMemory,
        'uptime': uptime,
        'isAuthenticated': isAuthenticated,
        'connectionState': connectionState.toString().split('.').last,
      };

  @override
  String toString() => 'DeviceModel(id: $id, name: $name, fw: $firmware, connected: $isConnected)';
}

List<FeatureModule> _parseFeatureModules(List<dynamic> modules) {
  return modules
      .map((m) => FeatureModule(
            name: m['name'] ?? '',
            description: m['description'] ?? '',
            endpoint: m['endpoint'] ?? '',
            enabled: m['enabled'] ?? false,
          )
      )
      .toList();
}

class FeatureModule {
  final String name;
  final String description;
  final String endpoint;
  final bool enabled;

  FeatureModule({
    required this.name,
    required this.description,
    required this.endpoint,
    required this.enabled,
  });
}