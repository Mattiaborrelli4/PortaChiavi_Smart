import 'package:flutter/material.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class RobotEyesScreen extends StatefulWidget {
  const RobotEyesScreen({super.key});

  @override
  State<RobotEyesScreen> createState() => _RobotEyesScreenState;
}

class _RobotEyesScreenState extends State<RobotEyesScreen> {
  final ApiClient _apiClient = ApiClient();
  Map<String, dynamic>? _robotState;
  bool _isLoading = true;
  String _selectedEmotion = 'neutral';

  @override
  void initState() {
    super.initState();
    _loadRobotState();
  }

  Future<void> _loadRobotState() async {
    try {
      final response = await _apiClient.get('/api/robot');
      setState(() {
        _robotState = response;
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
      appBar: AppBarWidget(title: 'RobotEyes'),
      body: _robotState == null
          ? const Center(child: Text('Nessuno stato disponibile'))
          : SingleChildScrollView(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  _buildStatusRow(),
                  const SizedBox(height: 24),
                  _buildEmotionControls(),
                  const SizedBox(height: 24),
                  _buildSleepControl(),
                  const SizedBox(height: 24),
                  _buildNavButton(context, 'Dashboard', '/dashboard'),
                ],
              ),
            ),
    );
  }

  Widget _buildStatusRow() {
    final emotions = _robotState?.keys.toList() ?? [];
    final hasEmotionSupport = emotions.isNotEmpty;
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'Stato RobotEyes',
          style: const TextStyle(fontSize: 18, color: Colors.white),
        ),
        const SizedBox(height: 8),
        Text(
          _robotState['ok'] == true ? 'Attivo' : 'Inattivo',
          style: TextStyle(color: _robotState['ok'] == true ? Colors.green : Colors.grey, fontSize: 16),
        ),
        if (!hasEmotionSupport) ...[
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(8),
            color: Colors.white24,
            child: const Text(
              'Nota: Comandi emotion non disponibili in firmware - usare /api/robot con action=emotion',
              style: TextStyle(color: Colors.white70, fontSize: 12),
            ),
          ),
        ],
      ],
    );
  }

  Widget _buildEmotionControls() {
    final emotions = ['happy', 'neutral', 'sad', 'angry', 'surprise', 'wink'];
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          'Emotion',
          style: TextStyle(fontSize: 16, color: Colors.white),
        ),
        const SizedBox(height: 8),
        Wrap(
          spacing: 8.0,
          children: emotions.map((emotion) {
            final isSelected = _selectedEmotion == emotion;
            return ElevatedButton.icon(
              onPressed: () async {
                final response = await _apiClient.setEmotion(emotion);
                if (response['error'] == null) {
                  setState(() => _selectedEmotion = emotion);
                  if (!context.mounted) return;
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Emotion inviata')),
                  );
                } else {
                  if (!context.mounted) return;
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(content: Text('Errore: ${response['error']}')),
                  );
                }
              },
              icon: Icon(isSelected ? Icons.star : Icons.remove, size: 16, color: Colors.white),
              label: Text(emotion[0].toUpperCase() + emotion.substring(1),
                  style: const TextStyle(color: Colors.white)),
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.primaries[
                    emotions.indexOf(emotion) % Colors.primaries.length].value,
              ),
            );
          }).toList(),
        ),
        const SizedBox(height: 8),
        if (!hasEmotionSupport) ...[
          Text(
            'Comandi emotion: non supportati direttamente dal firmware',
            style: TextStyle(color: Colors.white54, fontStyle: Italic, fontSize: 12),
          ),
          const SizedBox(height: 4),
          Text(
            'Usare POST /api/robot with action=emotion',
            style: TextStyle(color: Colors.white54, fontStyle: Italic, fontSize: 12),
          ),
        ],
      ],
    );
  }

  Widget _buildSleepControl() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          'Sleep Mode',
          style: TextStyle(fontSize: 16, color: Colors.white),
        ),
        const SizedBox(height: 8),
        SwitchListTile(
          title: const Text('Modalità sonno'),
          value: _robotState?.containsKey('sleep') == true,
          activeColor: Colors.amber,
          onChanged: (value) async {
            final response = await _apiClient.setSleepMode(value);
            if (response['error'] == null) {
              if (!context.mounted) return;
              ScaffoldMessenger.of(context).showSnackBar(
                const SnackBar(content: Text('Sleep mode updated')),
              );
            } else {
              if (!context.mounted) return;
              ScaffoldMessenger.of(context).showSnackBar(
                SnackBar(content: Text('Errore: ${response['error']}')),
              );
            }
          },
        ),
        if (!_robotState?.containsKey('sleep') == true) ...[
          const SizedBox(height: 4),
          Text(
            'Il firmware supporta sleep via POST /api/robot with action=sleep',
            style: TextStyle(color: Colors.white54, fontStyle: Italic, fontSize: 12),
          ),
        ],
      ],
    );
  }

  Widget _buildNavButton(BuildContext context, String label, String route) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: ElevatedButton.icon(
        onPressed: () => Navigator.pushNamed(context, route),
        icon: const Icon(Icons.arrow_forward, size: 14),
        label: Text(label),
      ),
    );
  }
}