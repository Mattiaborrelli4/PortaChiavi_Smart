import 'package:flutter/material.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';
import 'package:portachiave_mobile/core/utils/utils.dart';

class ConnectionScreen extends StatelessWidget {
  const ConnectionScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBarWidget(title: 'Connessione PortaChiave'),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            const Text(
              'Connect to PortaChiave device',
              style: TextStyle(fontSize: 18, color: Colors.white),
            ),
            const SizedBox(height: 24),
            TextField(
              decoration: const InputDecoration(
                labelText: 'IP Address',
                hintText: 'http://192.168.4.1',
              ),
            ),
            const SizedBox(height: 16),
            ElevatedButton(
              onPressed: () async {
                try {
                  final client = ApiClient();
                  final response = await client.getDeviceInfo();
                  if (response['id'] != null) {
                    if (!context.mounted) return;
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(content: Text('Device found!')),
                    );
                    Navigator.pushNamed(context, '/dashboard');
                  } else {
                    if (!context.mounted) return;
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(
                        content: Text('Device response invalid'),
                        backgroundColor: Colors.red,
                      ),
                    );
                  }
                } catch (e) {
                  if (!context.mounted) return;
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(
                      content: Text('Connection error: $e'),
                      backgroundColor: Colors.red,
                    ),
                  );
                }
              },
              child: const Text('Connetti'),
           ),
            const SizedBox(height: 16),
            const Text(
              'Indirizzo: 192.168.4.1',
              style: TextStyle(fontSize: 12, color: Colors.white54),
            ),
            const Text(
              'Questa rete potrebbe non avere accesso a Internet',
              style: TextStyle(fontSize: 12, color: Colors.white54),
            ),
          ],
        ),
      ),
    );
  }
}