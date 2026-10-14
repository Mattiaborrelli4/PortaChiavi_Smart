import 'package:flutter_test/flutter_test.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';

void main() {
  group('Connection state timeout tests') {
    test('disconnected state is initial', () {
      final model = ConnectionStateModel();
      expect(model.state, ConnectionState.disconnected);
    });

    test('connecting state transition', () {
      final model = ConnectionStateModel();
      model.setState(ConnectionState.connecting);
      expect(model.state, ConnectionState.connecting);
    });

    test('timeout state handling', () {
      final model = ConnectionStateModel();
      model.setState(ConnectionState.timeout, 'Connection timeout');
      expect(model.state, ConnectionState.timeout);
      expect(model.errorMessage, 'Connection timeout');
    });

    test('network error state', () {
      final model = ConnectionStateModel();
      model.setState(ConnectionState.networkError, 'No network available');
      expect(model.state, ConnectionState.networkError);
    });
  }
}