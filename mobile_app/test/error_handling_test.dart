import 'package:flutter_test/flutter_test.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/network/api_exception.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';

void main() {
  group('ApiClient error handling') {
    test('handles timeout errors', () {
      final client = ApiClient(
        baseUrl: 'http://192.168.4.1',
        timeout: Duration(milliseconds: 1),
      );
      expect(() => client.get('/api/device')).toThrow(ApiException);
    });

    test('handles connection errors', () {
      final client = ApiClient(
        baseUrl: 'http://192.168.4.1',
        timeout: const Duration(seconds: 1),
      );
      expect(() => client.get('/api/device')).toThrow(ApiException);
    });
  }

  group('ConnectionStateModel') {
    test('initial state is disconnected', () {
      final model = ConnectionStateModel();
      expect(model.state, ConnectionState.disconnected);
    });

    test('can update state', () {
      final model = ConnectionStateModel();
      model.setState(ConnectionState.connected, 'Connection successful');
      expect(model.state, ConnectionState.connected);
      expect(model.errorMessage, 'Connection successful');
    });

    test('reset returns to disconnected', () {
      final model = ConnectionStateModel();
      model.setState(ConnectionState.connected);
      model.reset();
      expect(model.state, ConnectionState.disconnected);
    });
  }
}