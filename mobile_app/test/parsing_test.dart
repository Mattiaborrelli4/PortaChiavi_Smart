import 'package:flutter_test/flutter_test.dart';
import 'package:portachiave_mobile/core/models/device_model.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

void main() {
  group('DeviceModel JSON parsing') {
    final json = {
      'id': 'PORTACHIAVE-001',
      'name': 'PortaChiave-001',
      'firmware': '1.2.3',
      'isConnected': true,
      'freeMemory': 2048,
      'uptime': 3600,
      'isAuthenticated': true,
      'connectionState': 'connected',
    };

    test('creates DeviceModel from valid JSON', () {
      final model = DeviceModel.fromJson(json);
      expect(model.id, json['id']);
      expect(model.name, json['name']);
      expect(model.firmware, json['firmware']);
      expect(model.isConnected, json['isConnected']);
      expect(model.freeMemory, json['freeMemory']);
      expect(model.uptime, json['uptime']);
      expect(model.isAuthenticated, json['isAuthenticated']);
      expect(model.connectionState, json['connectionState']);
    });
  }

  group('Utils formatting') {
    test('formatBytes with bytes', () {
      expect(Utils.formatBytes(0), '0 bytes');
      expect(Utils.formatBytes(1024), '1.00 KB');
      expect(Utils.formatBytes(1048576), '1.00 MB');
      expect(Utils.formatBytes(1073741824), '1.00 GB');
    });

    test('formatUptime with seconds', () {
      expect(Utils.formatUptime(0), '0 s');
      expect(Utils.formatUptime(30), '30 s');
      expect(Utils.formatUptime(60), '1 min 0 s');
      expect(Utils.formatUptime(120), '2 min 0 s');
      expect(Utils.formatUptime(3600), '1 h 0 min 0 s');
    });

    test('formatAuthStatus', () {
      expect(Utils.formatAuthStatus(true), 'Proprietario');
      expect(Utils.formatAuthStatus(false), 'Guest');
    });

    test('getConnectionStatusColor returns correct color', () {
      expect(Utils.getConnectionStatusColor(ConnectionState.connected), Colors.green);
      expect(Utils.getConnectionStatusColor(ConnectionState.disconnected), Colors.red);
      expect(Utils.getConnectionStatusColor(ConnectionState.connecting), Colors.orange);
    });
  }
}