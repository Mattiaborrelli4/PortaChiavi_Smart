import 'dart:convert';
import 'dart:math';

import 'package:portachiave_mobile/core/network/connection_state.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class ConnectionStateModel extends ChangeNotifier {
  ConnectionState _state = ConnectionState.disconnected;
  String? _errorMessage;
  String? _lastResponse;
  Duration? _lastResponseTime;

  ConnectionState get state => _state;
  String? get errorMessage => _errorMessage;
  String? get lastResponse => _lastResponse;
  Duration? get lastResponseTime => _lastResponseTime;

  setState(ConnectionState state, [String? errorMessage]) {
    _state = state;
    _errorMessage = errorMessage;
    notifyListeners();
  }

  void setResponse(dynamic data, [Duration? duration]) {
    final jsonString = data != null ? jsonEncode(data) : '';
    _lastResponse = jsonString.length > 200 ? '${jsonString.substring(0, 200)}...' : jsonString;
    _lastResponseTime = duration;
    notifyListeners();
  }

  void reset() => setState(ConnectionState.disconnected, null);

  bool get isConnected => _state == ConnectionState.connected;
  bool get isConnecting => _state == ConnectionState.connecting;
  bool get isDisconnected => _state == ConnectionState.disconnected;
  bool get hasError => _errorMessage != null;
}