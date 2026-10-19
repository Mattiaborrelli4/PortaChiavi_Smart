import 'package:flutter/material.dart';
import 'package:portachiave_mobile/core/network/api_client.dart';
import 'package:portachiave_mobile/core/network/connection_state.dart';

class AuthScreen extends StatefulWidget {
  const AuthScreen({super.key});

  @override
  State<AuthScreen> createState() => _AuthScreenState;
}

class _AuthScreenState extends State<AuthScreen> {
  final ApiClient _apiClient = ApiClient();
  bool _isLoggingIn = true;
  final _emailController = TextEditingController();
  final _passwordController = TextEditingController();
  bool _isLoading = false;
  String _status = 'In attesa di stato dal firmware';

  @override
  void initState() {
    super.initState();
    _loadAuthStatus();
  }

  Future<void> _loadAuthStatus() async {
    try {
      final response = await _apiClient.getAuthStatus();
      setState(() {
        _status = 'Ruolo: ${response['role'] ?? 'N/A'} | Stato: ${response['authenticated'] ?? false}';
      });
    } catch (e) {
      setState(() => _status = 'Errore nel recupero stato');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBarWidget(title: 'Autenticazione'),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            const Text(
              'Stato autenticazione dal firmware:',
              style: TextStyle(fontSize: 16, color: Colors.white70),
            ),
            const SizedBox(height: 8),
            Text(
              _status,
              style: const TextStyle(color: Colors.white, fontSize: 14),
            ),
            const Spacer(),
            if (!_isLoading) ...[
              const Text(
                'Accesso dispositivo: modalità sviluppatore',
                style: TextStyle(color: Colors.amber, fontStyle: Italic),
              ),
              const SizedBox(height: 16),
              ElevatedButton(
                onPressed: _login,
                child: const Text('Login'),
              ),
              const SizedBox(height: 8),
              ElevatedButton(
                onPressed: _logout,
                style: ElevatedButton.styleFrom(backgroundColor: Colors.grey),
                child: const Text('Logout'),
              ),
            ] ...[
              const Text(
                'Nessun endpoint login/logout configurato per questa build',
                style: TextStyle(color: Colors.white54, fontStyle: Italic),
              ),
            ],
            const SizedBox(height: 24),
            const Text(
              'Nota: l\'autenticazione usa endpoint /api/auth/status e /api/auth/login',
              style: TextStyle(fontSize: 12, color: Colors.white54),
            ),
          ],
        ),
      ),
    );
  }

  void _login() async {
    setState(() => _isLoading = true);
    try {
      final response = await _apiClient.login(_emailController.text, _passwordController.text);
      if (response['authenticated'] == true) {
        if (!context.mounted) return;
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('Login riuscito')),
        );
        Navigator.pushNamedAndRemoveUntil(context, '/dashboard', (route) => false);
      } else {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('Credenziali non valide'), backgroundColor: Colors.red),
        );
      }
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Errore login: $e'), backgroundColor: Colors.red),
      );
    } finally {
      setState(() => _isLoading = false);
    }
  }

  void _logout() async {
    try {
      await _apiClient.post('/api/auth/logout', {});
      if (!context.mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Logout riuscito')),
      );
      Navigator.pushNamedAndRemoveUntil(context, '/auth', (route) => false);
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Errore logout: $e'), backgroundColor: Colors.red),
      );
    }
  }
}