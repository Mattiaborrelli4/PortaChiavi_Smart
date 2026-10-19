import 'package:flutter/material.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class DeviceInfoScreen extends StatefulWidget {
  const DeviceInfoScreen({super.key});

  @override
  State<DeviceInfoScreen> createState() => _DeviceInfoScreenState;
}

class _DeviceInfoScreenState extends State<DeviceInfoScreen> {
  final ApiClient _apiClient = ApiClient();
  Map<String, dynamic>? _deviceInfo;
  bool _isLoading = true;

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
        _isLoading = false;
      });
    } catch (e) {
      setState(() => _isLoading = false);
      if (!context.mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Errore nel recupero informazioni')),
      );
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
      appBar: AppBarWidget(title: 'Informazioni Dispositivo'),
      body: _deviceInfo == null
          ? const Center(child: Text('Nessun dato disponibile'))
          : SingleChildScrollView(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  _buildDetailRow(
                    Icons.devices,
                    'Nome',
                    _deviceInfo['name'] ?? 'PortaChiave',
                  ),
                  _buildDetailRow(
                    Icons.id,
                    'ID',
                    _deviceInfo['id'] ?? 'N/A',
                  ),
                  _buildDetailRow(
                    Icons.info,
                    'Firmware',
                    _deviceInfo['firmware'] ?? 'N/A',
                  ),
                  _buildDetailRow(
                    Icons.timelapse,
                    'Uptime',
                    _formatUptime(_deviceInfo['uptime'] ?? 0),
                  ),
                  _buildDetailRow(
                    Icons.memory,
                    'Memoria libera',
                    _formatBytes(_deviceInfo['freeHeap'] ?? 0),
                  ),
                  _buildDetailRow(
                    Icons.wifi,
                    'Modalità WiFi',
                    (_deviceInfo['mode'] ?? 'N/A').toString(),
                  ),
                  _buildDetailRow(
                    Icons.ip_address,
                    'Indirizzo IP',
                    _deviceInfo['ip'] ?? 'N/A',
                  ),
                  _buildDetailRow(
                    Icons.signal_cellular_alt,
                    'Stato',
                    _deviceInfo['status'] ?? 'N/A',
                  ),
                ],
              ),
            ),
      ),
    );
  }

  Widget _buildDetailRow(IconData icon, String label, dynamic value) {
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
              value.toString(),
              style: const TextStyle(color: Colors.white, fontSize: 14),
              overflow: TextOverflow.fade,
            ),
          ),
        ],
      ),
    );
  }
}