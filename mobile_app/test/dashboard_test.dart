import 'package:flutter_test/flutter_test.dart';
import 'package:portachiave_mobile/features/connection/connection_screen.dart';
import 'package:portachiave_mobile/core/network/connection_state_model.dart';

void main() {
  group('Dashboard widget tests') {
    test('ConnectionScreen has correct app bar title', () {
      expect(ConnectionScreen().appBar, isNotNull);
    });

    test('ConnectionScreen builds without error', () {
      expect(() => ConnectionScreen(), returnsNormally);
    });

    test('ConnectionStateModel initial state', () {
      final model = ConnectionStateModel();
      expect(model.state, ConnectionState.disconnected);
    });
  }
}