import 'package:flutter/material.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  final ApiClient _apiClient = ApiClient();
  Map<String, dynamic>? _deviceInfo;
  bool _isLoading = true;
  String _firmware = 'N/A';
  int _uptime = 0;
  double _freeHeap = 0;
  bool _isAuthenticated = false;
  String _role = 'N/A';

  @override
  void initState() {
    super.initState();
    _loadDeviceInfo();
  }

  Future<void> _loadDeviceInfo() async {
    try {
      final response = await _apiClient.getDeviceInfo();
      setState(() {
        _deviceInfo = response;
        _firmware = _deviceInfo['firmware'] ?? 'N/A';
        _uptime = _deviceInfo['uptime'] ?? 0;
        _freeHeap = (_deviceInfo['freeHeap'] ?? 0).toDouble();
        _isAuthenticated = _deviceInfo['authenticated'] ?? false;
        _role = _deviceInfo['role'] ?? 'N/A';
        _isLoading = false;
      });
    } catch (e) {
      setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    if (_isLoading) {
      return const Scaffold(
        body: Center(child: CircularProgressIndicator()),
      );
    }

    return Scaffold(
      appBar: AppBarWidget(title: 'Dashboard PortaChiave'),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _buildInfoRow(
              Icons.devices,
              'Dispositivo',
              _deviceInfo['name'] ?? 'PortaChiave',
            ),
            _buildInfoRow(
              Icons.id,
              'ID',
              _deviceInfo['id'] ?? 'N/A',
            ),
            _buildInfoRow(
              Icons.info,
              'Firmware',
              _firmware,
            ),
            _buildInfoRow(
              Icons.timelapse,
              'Uptime',
              _formatUptime(_uptime),
            ),
            _buildInfoRow(
              Icons.memory,
              'Memoria libera',
              _formatBytes(_freeHeap),
            ),
            if (_isAuthenticated) ...[
              _buildInfoRow(
                Icons.security,
                'Ruolo',
                _role,
              ),
              _buildInfoRow(
                Icons.check_circle,
                'Autenticazione',
                'Attiva',
              ),
            ] ...[
              _buildInfoRow(
                Icons.lock_outline,
                'Autenticazione',
                'Non autenticato',
              ),
            ],
            const SizedBox(height: 24),
            ElevatedButton.icon(
              onPressed: _loadDeviceInfo,
              icon: const Icon(Icons.refresh),
              label: const Text('Aggiorna'),
            ),
            const SizedBox(height: 16),
            _buildNavButton(
              context,
              'Dispositivo',
              '/device-info',
              Icons.device_hub,
            ),
            _buildNavButton(
              context,
              'RobotEyes',
              '/robot-eyes',
              Icons.remove_red_eye,
            ),
            _buildNavButton(
              context,
              'Impostazioni',
              '/settings',
              Icons.settings,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildInfoRow(IconData icon, String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: Row(
        children: [
          Icon(icon, color: Colors.white70, size: 20),
          const SizedBox(width: 8),
          Text(
            '$label: ',
            style: const TextStyle(color: Colors.white70, fontSize: 14),
          ),
          Expanded(
            child: Text(
              value,
              style: const TextStyle(color: Colors.white, fontSize: 14),
              overflow: TextOverflow.fade,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildNavButton(
    BuildContext context,
    String label,
    String route,
    IconData icon,
  ) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: ElevatedButton.icon(
        onPressed: () => Navigator.pushNamed(context, route),
        icon: Icon(icon, size: 16),
        label: Text(label),
      ),
    );
  }
}