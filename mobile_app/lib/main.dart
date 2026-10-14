import 'package:flutter/material.dart';
import 'package:portachiave_mobile/app/router.dart';
import 'package:portachiave_mobile/app/theme.dart';

void main() => runApp(const PortaChiaveApp());

class PortaChiaveApp extends StatelessWidget {
  const PortaChiaveApp({super.key});

  @override
  Widget build(BuildContext context) => MaterialApp.router(
    title: 'PortaChiave',
    theme: themeData(),
    darkTheme: themeData(),
    themeMode: ThemeMode.dark,
    routerDelegate: routerDelegate(),
    routeInformationProvider: routeInformationProvider(),
    routeInformationParser: routeInformationParser(),
    routeInformationListener: routeInformationListener(),
  );
}