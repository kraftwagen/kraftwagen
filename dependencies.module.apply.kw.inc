<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-apply-module-dependencies`.
 */

/**
 * Implements drush_COMMAND for `drush kw-apply-module-dependencies`.
 *
 * @param string $environment
 *   The name of the environment. Defaults to 'production'.
 */
function drush_kraftwagen_kw_apply_module_dependencies($environment = 'production') {
  // Enable watchdog printing to show information about enabled, disabled,
  // installed and uninstalled modules.
  drush_set_option('watchdog', 'print');

  // Get the data we need. This is a excerpt of system_rebuild_module_data with
  // the environment specific dependencies logic inserted.
  $modules = _system_rebuild_module_data();
  ksort($modules);
  system_get_files_database($modules, 'module');
  system_update_files_database($modules, 'module');

  // Inject enviroment specific dependencies into the dependencies.
  foreach ($modules as $key => $module) {
    if (!empty($module->info['env_dependencies'][$environment])) {
      foreach ($module->info['env_dependencies'][$environment] as $dependency) {
        $module->info['dependencies'][] = $dependency;
      }
    }
  }

  // Resolve dependencies and find out in which order to install modules.
  $files = _module_build_dependencies($modules);

  // Get the active profile name.
  $profile = drupal_get_profile();

  // If we can't gather data about the install profile, we can't do anything.
  if (!isset($files[$profile])) {
    return drush_set_error(dt('Could not find dependencies for profile !profile.', array('!profile' => $profile)));
  }

  // To find the required modules we need to clear the required flag on the
  // profile, because otherwise all modules that need to be enabled are called
  // required. Required is reserved for base modules for Drupal.
  $files[$profile]->info['required'] = FALSE;

  // Find the 'required' modules
  // Noteworthy fact when debugging: the field module markes modules that are
  // used by existing fields as 'required'.
  $required = array();
  foreach ($files as $name => $info) {
    if (!empty($info->info['required']) && !isset($required[$name])) {
      $required[$name] = $info->sort;
      if (!empty($info->requires)) {
        foreach (array_keys($info->requires) as $dependency) {
          if (!isset($required[$dependency])) {
            if (!isset($files[$dependency])) {
              return drush_set_error(dt('Dependency !dependency of required module !module does not exist.', array('!module' => $name, '!dependency' => $dependency)));
            }
            $required[$dependency] = $files[$dependency]->sort;
          }
        }
      }
    }
  }
  arsort($required);

  // Find the modules that the profile depend on (the whole dependency tree).
  $dependencies = array();
  foreach (array_keys($files[$profile]->requires) as $dependency) {
    if (!isset($required[$dependency])) {
      if (!isset($files[$dependency])) {
        return drush_set_error(dt('Dependency !dependency of the install profile !profile does not exist.', array('!dependency' => $dependency, '!profile' => $profile)));
      }

      $dependencies[$dependency] = $files[$dependency]->sort;
    }
  }
  arsort($dependencies);

  // Enable the required modules and dependencies.
  $expected = array_merge(array_keys($required), array_keys($dependencies));
  $result = module_enable($expected);
  if (!$result) {
    return drush_set_error(dt('An unexpected error occured while trying to enable modules !modules', array('!modules', implode(', ', $expected))));
  }

  // Check for enabled modules that are not expected to be enabled.
  $unexpected = array();
  foreach (module_list(TRUE) as $module) {
    if (!in_array($module, $expected) && $module != $profile) {
      if (!isset($files[$module])) {
        drush_log(dt('Could not find dependencies for currently enabled module !module.', array('!module' => $module)), 'error');
      }
      $unexpected[$module] = $files[$module]->sort;
    }
  }
  asort($unexpected);
  $unexpected = array_keys($unexpected);

  // Disable the unexpected modules.
  if ($unexpected) {
    module_disable($unexpected);
  }

  // Check for modules that are still installed even though they are not
  // expected.
  $orphan_installs = array();
  include_once DRUPAL_ROOT . '/includes/install.inc';
  foreach ($files as $name => $info) {
    if ($name != $profile && !in_array($name, $expected)) {
      if (drupal_get_installed_schema_version($name) != SCHEMA_UNINSTALLED) {
        $orphan_installs[$name] = $info->sort;
      }
    }
  }
  asort($orphan_installs);
  $orphan_installs = array_keys($orphan_installs);

  // Remove the hanging installs.
  if ($orphan_installs) {
    $result = drupal_uninstall_modules($orphan_installs, FALSE);
    if (!$result) {
      return drush_set_error(dt('An unexpected error occured while trying to uninstall modules !modules', array('!modules', implode(', ', $orphan_installs))));
    }
  }

  drush_log(dt('Finished enabling required modules and uninstalling orphan modules.'), 'ok');
}
