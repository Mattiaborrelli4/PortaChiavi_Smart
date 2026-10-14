import 'package:go_router/go_router.dart';
import 'features/onboarding/onboarding_screen.dart';
import 'features/connection/connection_screen.dart';
import 'features/dashboard/dashboard_screen.dart';
import 'features/device_info/device_info_screen.dart';
import 'features/authentication/auth_screen.dart';
import 'features/robot_eyes/robot_eyes_screen.dart';
import 'features/settings/settings_screen.dart';

final goRouterConfig = [
  GoRoute(
    path: '/onboarding',
    builder: (context, state) => const OnboardingScreen(),
    pageBuilder: (context, state) => const MaterialPage(
      child: OnboardingScreen(),
    ),
  ),
  GoRoute(
    path: '/connection',
    builder: (context, state) => const ConnectionScreen(),
    pageBuilder: (context, state) => const MaterialPage(
      child: ConnectionScreen(),
    ),
  ),
  GoRoute(
    path: '/dashboard',
    builder: (context, state) => const DashboardScreen(),
    pageBuilder: (context, state) => const MaterialPage(
      child: DashboardScreen(),
    ),
  ),
  GoRoute(
    path: '/device-info',
    builder: (context, state) => const DeviceInfoScreen(),
    pageBuilder: (context, state) => const MaterialPage(
      child: DeviceInfoScreen(),
    ),
  ),
  GoRoute(
    path: '/auth',
    builder: (context, state) => const AuthScreen(),
    pageBuilder: (context, state) => const MaterialPage(
      child: AuthScreen(),
    ),
  ),
  GoRoute(
    path: '/robot-eyes',
    builder: (context, state) => const RobotEyesScreen(),
    pageBuilder: (context, state) => const MaterialPage(
      child: RobotEyesScreen(),
    ),
  ),
  GoRoute(
    path: '/settings',
    builder: (context, state) => const SettingsScreen(),
    pageBuilder: (context, state) => const MaterialPage(
      child: SettingsScreen(),
    ),
  ),
];