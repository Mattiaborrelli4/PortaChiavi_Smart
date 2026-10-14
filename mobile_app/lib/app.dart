import 'package:flutter/material.dart';
import 'router.dart';

final navigatorKey = GlobalKey<NavigatorState>();

AppRouterDelegate routerDelegate() => AppRouterDelegate(navigatorKey);
RouteInformationProvider routeInformationProvider() =>
    RouteInformationProvider(navigatorKey);
RouteInformationParser routeInformationParser() =>
    RouteInformationParser(navigatorKey);
RouteInformationListener routeInformationListener() =>
    RouteInformationListener(navigatorKey);