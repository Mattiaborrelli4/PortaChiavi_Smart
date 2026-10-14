import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:portachiave_mobile/app/router/go_router_config.dart';

final navigatorKey = GlobalKey<NavigatorState>();

GoRouterDelegate routerDelegate() => _goRouterDelegate;

final _goRouterDelegate = GoRouter(
  navigatorKey: navigatorKey,
  initialLocation: '/onboarding',
  routes: goRouterConfig(),
);

RouteInformationProvider routeInformationProvider() =>
    RouteInformationProvider(navigatorKey);

RouteInformationParser routeInformationParser() =>
    _GoRouteInformationParser();

RouteInformationListener routeInformationListener() =>
    RouteInformationListener(navigatorKey);