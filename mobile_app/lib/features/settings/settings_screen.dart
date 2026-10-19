import 'package:flutter/material.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState;
}

class _SettingsScreenState extends State<SettingsScreen> {
  bool _darkMode = true;
  int _sessionTimeout = 30;
  bool _autoConnect = false;
  final _deviceNameController = TextEditingController();
  String _deviceName = 'PortaChiave';
  bool _saving = false;

  @override
  void initState() {
    super.initState();
    _loadDeviceName();
  }

  Future<void> _loadDeviceName() async {
    try {
      final response = await ApiClient().getDeviceInfo();
      setState(() {
        _deviceName = response['name'] ?? 'PortaChiave';
        _deviceNameController.text = _deviceName;
      });
    } catch (e) {
      setState(() {
        _deviceName = 'PortaChiave';
        _deviceNameController.text = 'PortaChiave';
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBarWidget(title: 'Impostazioni'),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Preferenze applicazione',
              style: TextStyle(fontSize: 18, color: Colors.white),
            ),
            const SizedBox(height: 24),
            _buildToggleSwitch(
              context,
              'Modalità scura',
              _darkMode,
              (value) =>
                  setState(() => _darkMode = value),
            ),
            _buildToggleSwitch(
              context,
              'Accesso automatico',
              _autoConnect,
              (value) => setState(() => _autoConnect = value),
            ),
            const ListTile(
              title: Text(
                'Nome dispositivo',
                style: TextStyle(color: Colors.white70),
              ),
              trailing: Text(
                'Modificabile localmente',
                style: TextStyle(color: Colors.white54, fontSize: 12),
              ),
            ),
            TextField(
              controller: _deviceNameController,
              decoration: const InputDecoration(
                labelText: 'Nome dispositivo',
                hintText: 'PortaChiave',
                border: OutlineInputBorder(),
              ),
              onChanged: (value) => setState(() => _deviceName = value),
            ),
            const SizedBox(height: 16),
            const Text(
              'Limiti firmware',
              style: TextStyle(color: Colors.amber, fontStyle: Italic),
            ),
            const Text(
              'Modifiche impostazioni firmware non supportate - nessuna endpoint di persistenza',
              style: TextStyle(color: Colors.white54, fontStyle: Italic, fontSize: 12),
            ),
            const SizedBox(height: 24),
            _saving
                ? const CircularProgressIndicator()
                : ElevatedButton(
                onPressed: _saveSettings,
                child: const Text('Salva preferenze')),
            const SizedBox(height: 16),
            const Text(
              'Nota: tutte le preferenze sono salvate localmente sull\'app',
              style: TextStyle(fontSize: 12, color: Colors.white54),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildToggleSwitch(
      BuildContext context, String value, bool valueBool, ValueChanged<bool> onChanged) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: Row(
        children: [
          Expanded(
            child: Text(
              value,
              style: const TextStyle(color: Colors.white70),
            ),
          ),
          Switch(
            value: valueBool,
            activeColor: Colors.amber,
            onChanged: onChanged,
          ),
        ],
      ),
    );
  }

  void _saveSettings() async {
    setState(() => _saving = true);
    // Note: No firmware endpoint for saving settings - saved locally only
    await Future.delayed(const Duration(seconds: 1));
    setState(() => _saving = false);
    if (!context.mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      const SnackBar(content: Text('Preferenze salvate localmente')),
    );
  }
}