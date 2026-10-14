import 'package:flutter/material.dart';

class _GoRouteInformationParser extends RouteInformationParser<GoRouteInfo> {
  @override
  Future<GoRouteInfo> parseRouteInformation(
      RouteInformation configuration) async {
    final uri = Uri.parse(configuration.location);
    final match = GoRouteInfo.tryParse(uri);
    return match ?? GoRouteInfo(	name: '/onboarding',
      location: '/onboarding',
      revision: 0,
    );
  }

  @override
  RouteInformation serializeRouteInformation(GoRouteInfo route) => RouteInformation(
    location: route.location,
    uri: Uri.parse(route.location),
  );
}

class GoRouteInfo {
  final String name;
  final String location;
  final int revision;

  GoRouteInfo({
    required this.name,
    required this.location,
    required this.revision,
  })

  GoRouteInfo.tryParse(Uri uri) {
    final path = uri.path.isNotEmpty ? uri.path : '/onboarding';
    switch (path) {
      case '/onboarding':
        return GoRouteInfo(name: '/onboarding', location: '/onboarding', revision: 0);
      case '/connection':
        return GoRouteInfo(name: '/connection', location: '/connection', revision: 0);
      case '/dashboard':
        return GoRouteInfo(name: '/dashboard', location: '/dashboard', revision: 0);
      case '/device-info':
        return GoRouteInfo(name: '/device-info', location: '/device-info', revision: 0);
      case '/auth':
        return GoRouteInfo(name: '/auth', location: '/auth', revision: 0);
      case '/robot-eyes':
        return GoRouteInfo(name: '/robot-eyes', location: '/robot-eyes', revision: 0);
      case '/settings':
        return GoRouteInfo(name: '/settings', location: '/settings', revision: 0);
      default:
        return null;
    }
  }
}