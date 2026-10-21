import 'dart:convert';

import 'package:dio/dio.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class ApiClient {
  final Dio _dio;
  final String _baseUrl;
  final Duration _timeout;
  final ConnectionStateModel _connectionState;

  ApiClient({
    String? baseUrl,
    Duration? timeout,
    ConnectionStateModel? connectionState,
  })  : _baseUrl = baseUrl ?? DeviceConfig.baseUrl,
         _timeout = timeout ?? DeviceConfig.timeout,
         _connectionState = connectionState ?? ConnectionStateModel() {

    _initDio();
    _initInterceptors();
  }

  Dio get dio => _dio;

  void _initDio() {
    _dio = Dio(BaseOptions(
      baseUrl: _baseUrl,
      connectTimeout: _timeout,
      receiveTimeout: _timeout,
      sendTimeout: _timeout,
    ));
  }

  void _initInterceptors() {
    _dio.interceptors.add(InterceptorsWrapper(
      onRequest: (options, handler) {
        _connectionState.setState(ConnectionState.connecting);
        handler.next(options);
      },
      onResponse: (response, handler) {
        _connectionState.setState(ConnectionState.connected);
        handler.next(response);
      },
      onError: (DioError error, handler) {
        _handleDioError(error);
        _connectionState.setState(ConnectionState.networkError);
        handler.next(error);
      },
    ));
  }

  Map<String, dynamic> _handleResponse(Map<String, dynamic> response) {
    // Firmware returns direct JSON without success/data envelope
    return response;
  }

  void _handleDioError(DioError error) {
    switch (error.type) {
      case DioErrorType.connectionTimeout:
        _connectionState.setState(ConnectionState.timeout, 'Timeout connecting to device');
        break;
      case DioErrorType.sendTimeout:
        _connectionState.setState(ConnectionState.timeout, 'Send timeout');
        break;
      case DioErrorType.receiveTimeout:
        _connectionState.setState(ConnectionState.timeout, 'Receive timeout');
        break;
      case DioErrorType.connection:
        _connectionState.setState(ConnectionState.disconnected, 'No connection to device');
        break;
      case DioErrorType.receive:
        _connectionState.setState(ConnectionState.apiError, 'Error receiving data');
        break;
      case DioErrorType.send:
        _connectionState.setState(ConnectionState.apiError, 'Error sending data');
        break;
      default:
        _connectionState.setState(ConnectionState.networkError, error.message);
        break;
    }
  }

  Future<Map<String, dynamic>> get(String endpoint) async {
    try {
      final response = await _dio.get<String, dynamic>(endpoint);
      return _handleResponse(response.data as Map<String, dynamic>);
    } on DioException catch (e) {
      _handleDioError(e);
      throw ApiException(e.message ?? 'Unknown error', e.code);
    }
  }

  Future<Map<String, dynamic>> post(String endpoint, Map<String, dynamic> data) async {
    try {
      final response = await _dio.post<String, dynamic>(endpoint, data: data);
      return _handleResponse(response.data as Map<String, dynamic>);
    } on DioException catch (e) {
      _handleDioError(e);
      throw ApiException(e.message ?? 'Unknown error', e.code);
    }
  }

  Future<Map<String, dynamic>> put(String endpoint, Map<String, dynamic> data) async {
    try {
      final response = await _dio.put<String, dynamic>(endpoint, data: data);
      return _handleResponse(response.data as Map<String, dynamic>);
    } on DioException catch (e) {
      _handleDioError(e);
      throw ApiException(e.message ?? 'Unknown error', e.code);
    }
  }

  Future<Map<String, dynamic>> delete(String endpoint) async {
    try {
      final response = await _dio.delete<String, dynamic>(endpoint);
      return _handleResponse(response.data as Map<String, dynamic>);
    } on DioException catch (e) {
      _handleDioError(e);
      throw ApiException(e.message ?? 'Unknown error', e.code);
    }
  }

  // Device endpoints
  Future<Map<String, dynamic>> getDeviceInfo() async {
    return await get('/api/device');
  }

  // System endpoints
  Future<Map<String, dynamic>> getSystemInfo() async {
    return await get('/api/system');
  }

  // Authentication endpoints
  Future<Map<String, dynamic>> getAuthStatus() async {
    return await get('/api/auth/status');
  }

  Future<Map<String, dynamic>> login(String email, String password) async {
    return await post('/api/auth/login', {'email': email, 'password': password});
  }

  // Robot eyes endpoints - use /api/robot with action parameter
  // Firmware supports: POST /api/robot with action=emotion/sleep/anim
  Future<Map<String, dynamic>> setEmotion(String emotion) async {
    return await post('/api/robot', {'action': 'emotion', 'value': emotion});
  }

  Future<Map<String, dynamic>> setSleepMode(bool sleepMode) async {
    return await post('/api/robot', {'action': 'sleep', 'value': sleepMode ? 'on' : 'off'});
  }

  // Animation endpoint not in firmware - disabled
  Future<Map<String, dynamic>> setAnimation(int animationIndex) async {
    return {'error': 'Animation endpoint not supported by firmware'};
  }

  // Network endpoints
  Future<Map<String, dynamic>> scanWifi() async {
    return await get('/api/wifi/scan');
  }

  // scanNetworksDeep disabled - not for production use
  Future<Map<String, dynamic>> scanNetworksDeep() async {
    return {'error': 'Network scanning not available in app'};
  }

  // Evil twin endpoint disabled - security risk
  Future<Map<String, dynamic>> startEvilTwin(String ssid, String password) async {
    return {'error': 'Evil twin not supported - use direct device connection'};
  }

  // Scanner endpoints disabled
  Future<Map<String, dynamic>> startScanner() async {
    return {'error': 'Scanner not supported by app'};
  }

  // Camera endpoints - firmware has /api/device/camera/status
  Future<Map<String, dynamic>> checkCamera() async {
    return await get('/api/device/camera/status');
  }

  // Wardrive endpoints disabled - security risk
  Future<Map<String, dynamic>> startWardrive() async {
    return {'error': 'Wardriving not supported in app'};
  }
}

class DeviceConfig {
  static const String baseUrl = 'http://192.168.4.1';
  static const Duration timeout = Duration(seconds: 5);
}

class ApiException implements Exception {
  final String message;
  final String? code;
  final dynamic args;

  ApiException(this.message, [this.code, this.args]);

  @override
  String toString() => 'ApiException: $message';
}