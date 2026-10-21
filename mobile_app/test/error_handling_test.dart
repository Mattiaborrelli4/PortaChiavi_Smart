import 'package:flutter_test/flutter_test.dart';
import 'package:portachiave_mobile/core/network/api_exception.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';

void main() {
  group('ApiException tests') {
    test('create ApiException with message', () {
      final exception = ApiException('Test error');
      expect(exception.message, 'Test error');
    });

    test('create ApiException with message and code', () {
      final exception = ApiException('Test error', 'E001');
      expect(exception.message, 'Test error');
      expect(exception.code, 'E001');
    });

    test('ApiException implements Exception', () {
      final exception = ApiException('Test error');
      expect(exception.toString(), contains('ApiException'));
    });
  }

  group('ConnectionState tests') {
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

    test('hasError property', () {
      final model = ConnectionStateModel();
      expect(model.hasError, isFalse);
      model.setState(ConnectionState.disconnected, 'Some error');
      expect(model.hasError, isTrue);
    });
  }
}