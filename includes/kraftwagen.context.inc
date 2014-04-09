<?php

/**
 * @file
 * This file contains the functions that we need to manage the Kraftwagen
 * context.
 */

// Name of the Kraftwagen configuration file.
define('KRAFTWAGEN_RC', 'kraftwagenrc.php');

// Prefix of all Kraftwagen configuration values.
define('KRAFTWAGEN_PREFIX', 'kw-');

// Drush context names where Kraftwagen context values should be stored.
// 'default' is actually intended for the way we use it, but 'user' is not.
// Because the context array of drush is not extendable, and we need to load our
// values somewhere, we had to choose from the existing contexts.
// We needed a place that overrides the values in ~/.drushrc.php, but does not
// override the values in a Drupal installation. Because ~/.drushrc.php is
// already loaded into 'user' before we load a kraftwagenrc.php, we can savely
// store our values in the 'user' context.
define('KRAFTWAGEN_CONTEXT_DEFAULT', 'default');
define('KRAFTWAGEN_CONTEXT_ROOT', 'user');
define('KRAFTWAGEN_CONTEXT_RUNTIME', 'process');

/**
 * Load the default kraftwagenrc.php file.
 */
function kraftwagen_context_load_default_config() {
  foreach (array(dirname(__FILE__) . '/../' . KRAFTWAGEN_RC, '/etc/' . KRAFTWAGEN_RC, drush_server_home() . '/.drush/' . KRAFTWAGEN_RC) as $file) {
    if (file_exists($file)) {
      kraftwagen_context_load_config_file($file, KRAFTWAGEN_CONTEXT_DEFAULT);
    }
  }
}

/**
 * Initialize the Kraftwagen root and load the kraftwagenrc.php file in it, if it
 * exists.
 */
function kraftwagen_context_init_kraftwagen_root($required = TRUE) {
  // Try to find the root.
  $root = kraftwagen_context_find_kraftwagen_root(getcwd());

  // If a root is found, initialize it.
  if (!empty($root)) {
    kraftwagen_context_set_option('root-path', $root);
    foreach (array($root . '/cnf/' . KRAFTWAGEN_RC, $root . '/' . KRAFTWAGEN_RC) as $file) {
      if (file_exists($file)) {
        kraftwagen_context_load_config_file($file, KRAFTWAGEN_CONTEXT_ROOT);
      }
    }
  }
  elseif ($required) {
    drush_set_error('kraftwagen', dt('Could not find a Kraftwagen root'));
  }
}

/**
 * Traverse upwards in the directory tree to find a Kraftwagen working directory.
 *
 * @param $target
 *   Path to start the traversal
 * @return
 *   The path of the Kraftwagen root, or FALSE if no root is found.
 */
function kraftwagen_context_find_kraftwagen_root($target) {
  // Make sure we have a full and valid directory path to start with.
  $dir = realpath($target);
  if (is_file($dir)) {
    $dir = dirname($dir);
  }
  if (!is_dir($dir)) {
    return FALSE;
  }

  // Traverse upwards.
  while ($dir) {
    // If the directory is a valid kraftwagen root, return it.
    if (kraftwagen_context_check_directory($dir)) {
      return $dir;
    }

    // Try to find the parent directory.
    $prev_dir = $dir;
    $dir = dirname($dir);

    // If we are at the root. dirname('/') == '/'.
    if ($dir == $prev_dir) {
      return FALSE;
    }
  }

  return FALSE;
}

/**
 * Internal helper function. preg_replace callback to replace for example
 * '*src_dir*' with the expected path of the source dir from Kraftwagen context
 * options.
 */
function _kraftwagen_context_check_directory_replace($matches) {
  $value = kraftwagen_context_get_option($matches[1]);
  return $value ? $value : '*ERROR*';
}

/**
 * Check if a directory is Kraftwagen root directory
 *
 * @param $path
 *   The path to check for
 * @return
 *   TRUE if the path is Kraftwagen root directory, FALSE if not.
 */
function kraftwagen_context_check_directory($path) {
  // Check if a kraftwagen.rc file is in the directory. If it is, load it
  // temporarily.
  $rc_file = FALSE;
  if (is_file($path . '/' . KRAFTWAGEN_RC)) {
    kraftwagen_context_load_config_file_temp($path . '/' . KRAFTWAGEN_RC);
    $rc_file = TRUE;
  }

  $return = TRUE;

  // Perform the root checks.
  $root_checks = kraftwagen_context_get_option('root-checks');
  foreach ($root_checks as $check) {
    $func = 'kraftwagen_context_check_directory_' . $check['type'];
    if (function_exists($func)) {
      // Manage the function parameters. '*OPTION*' is replaced by the result of
      // kraftwagen_context_get_option('OPTION').
      $parameters = array();
      if (!empty($check['parameters']) && is_array($check['parameters'])) {
        foreach ($check['parameters'] as $key => $value) {
          $parameters[$key] = is_string($value) ? preg_replace_callback('/\*([^*]+)\*/', '_kraftwagen_context_check_directory_replace', $value) : $value;
          if (preg_match('/\*[^*]+\*/', $parameters[$key])) {
            return FALSE;
          }
        }
      }

      $result = $func($path, $parameters);
      if (!$result) {
        $return = FALSE; break;
      }
    }
    else {
      $return = FALSE; break;
    }
  }

  if ($rc_file) {
    kraftwagen_context_restore_config();
  }

  return $return;
}

/**
 * Kraftwagen root check callback for 'require_directory'. Checks if a directory
 * exists.
 */
function kraftwagen_context_check_directory_require_directory($dir, $parameters) {
  return is_dir($dir . '/' . $parameters['dir']);
}

/**
 * Kraftwagen root check callback for 'require_file'. Checks if a file exists.
 */
function kraftwagen_context_check_directory_require_file($dir, $parameters) {
  return is_file($dir . '/' . $parameters['file']);
}

/**
 * Get the value of Kraftwagen option.
 *
 * A fairly simple but very convenient wrapper around drush_get_option().
 *
 * @param $key
 *   The name of the Kraftwagen option to get
 * @param $default
 *   Optional. The value to return if the option has not been set
 * @param $context
 *   Optional. The context to check for the option. If this is set, only this
 *   context will be searched.
 *
 * @return
 *   The value of the Kraftwagen option, or it is not set the value of $default,
 *   which defaults to NULL.
 */
function kraftwagen_context_get_option($key, $default = NULL, $context = NULL) {
  return drush_get_option(KRAFTWAGEN_PREFIX . $key, $default, $context);
}

/**
 * Get the values of one or more kraftwagen options.
 *
 * @param $key1
 * @param $key2
 * @param ...
 *   The Kraftwagen options to get.
 *
 * @return
 *   Array with the return values for kraftwagen_context_get_option for every
 *   option.
 */
function kraftwagen_context_get_options() {
  return array_map('kraftwagen_context_get_option', func_get_args());
}

/**
 * Set an Kraftwagen option in one of the option contexts.
 *
 * A fairly simple but very convenient wrapper around drush_set_option().
 *
 * @param $key
 *   The Kraftwagen option to set.
 * @param $value
 *   The value to set it to.
 * @param $context
 *   Optional. Which context to set it in.
 * @return
 *   The return value of drush_set_option, which is the $value parameter.
 */
function kraftwagen_context_set_option($key, $value, $context = KRAFTWAGEN_CONTEXT_RUNTIME) {
  return drush_set_option(KRAFTWAGEN_PREFIX . $key, $value, $context);
}

/**
 * Load a kraftwagen config file.
 *
 * @param $config
 *   The location of the configuration file.
 * @param $context
 *   Optional. The context where the file should be loaded to.
 *
 * @return
 *   On error FALSE, otherwise TRUE.
 */
function kraftwagen_context_load_config_file($config, $context = KRAFTWAGEN_CONTEXT_RUNTIME) {
  if (!file_exists($config)) {
    return drush_set_error(dt('Kraftwagen config file !config could not be found.', array('!config' => realpath($config))));
  }

  // Fetch options from file.
  $options = array();
  drush_log(dt('Loading kraftwagenrc "!config".', array('!config' => realpath($config))), 'kraftwagen');
  $ret = @include($config);
  if ($ret === FALSE) {
    return drush_set_error(dt('Kraftwagen config file !config could not be opened.', array('!config' => realpath($config))));
  }

  // Load options into context.
  if (!empty($options)) {
    $drush_options = array();
    foreach ($options as $key => $value) {
      $drush_options[KRAFTWAGEN_PREFIX . $key] = $value;
    }
    $drush_options = array_merge(drush_get_context($context), $drush_options);
    drush_set_config_options($context, $drush_options, array());
  }

  return TRUE;
}

/**
 * Get a key => value array of all available Kraftwagen config options.
 *
 * @return
 *   Associative array of Kraftwagen config options.
 */
function kraftwagen_context_get_config_options() {
  $options = _kraftwagen_context_get_config_options();

  $result = array();
  foreach (drush_context_names() as $context) if (!empty($options[$context])) {
    $result = array_merge($result, $options[$context]);
  }

  return $result;
}

/**
 * Temporarily load Kraftwagen config file
 *
 * This allows for loading a config file while searching the Kraftwagen root. It
 * makes it possible to load a config file, using the values from the config
 * file with all the normal functions for accessing config values and then
 * settings the config values back to the values they had before loading in the
 * config file.
 *
 * @param $config
 *   The config file to load.
 *
 * @return
 *   On error FALSE, otherwise TRUE.
 */
function kraftwagen_context_load_config_file_temp($config) {
  $data = _kraftwagen_context_config_store();

  // Refuse to load the file, if another file is currently temporarily loaded.
  if ($data !== FALSE) {
    return drush_set_error('kraftwagen', dt('Could not load temporary config file !config, because another temporary config file is currently loaded.', array('!config' => $config)));
  }

  // Get the current config data and store it.
  $data = _kraftwagen_context_get_config_options();
  _kraftwagen_context_config_store($data);

  // Load the config file.
  return kraftwagen_context_load_config_file($config);
}

/**
 * Restore the config values that were stored while loading a temporary config
 * file.
 */
function kraftwagen_context_restore_config() {
  $data = _kraftwagen_context_config_store();

  foreach(drush_context_names() as $context) {
    $cache = &drush_get_context($context);
    if (is_array($cache)) {

      // Remove the items that were not present before loading the file.
      $keys = array_keys($cache);
      foreach ($keys as $key) {
        if (substr($key, 0, strlen(KRAFTWAGEN_PREFIX)) == KRAFTWAGEN_PREFIX) {
          if (!isset($data[$context][substr($key, strlen(KRAFTWAGEN_PREFIX))])) {
            unset($cache[$key]);
          }
        }
      }

      // Set the values from the storage.
      if (!empty($data[$context])) {
        foreach ($data[$context] as $key => $value) {
          $cache[KRAFTWAGEN_PREFIX . $key] = $value;
        }
      }
    }
  }

  // Empty the storage.
  _kraftwagen_context_config_store(FALSE);
}

/**
 * Interal helper function. Get all Kraftwagen config options per context.
 *
 * @return
 *   Associative array, per context an associative array of Kraftwagen options.
 */
function _kraftwagen_context_get_config_options() {
  $result = array();
  foreach (drush_context_names() as $context) {
    $options = drush_get_context($context);
    $result[$context] = array();
    if (is_array($options)) foreach ($options as $key => $value) {
      if (substr($key, 0, strlen(KRAFTWAGEN_PREFIX)) == KRAFTWAGEN_PREFIX) {
        $result[$context][substr($key, strlen(KRAFTWAGEN_PREFIX))] = $value;
      }
    }
  }
  return array_filter($result);
}

/**
 * Internal helper function. Store a config array temporarily.
 */
function _kraftwagen_context_config_store($data = NULL) {
  static $store = FALSE;

  if (isset($data)) {
    $store = $data;
  }

  return $store;
}

/**
 * Wrapper around drush_confirm that does not ask the question if the a
 * Kraftwagen option is set.
 *
 * @param $option
 *   The Kraftwagen option that can be used as default value.
 * @param $msg
 *   The question to ask
 */
function kraftwagen_context_confirm($option, $msg) {
  // Use default value for option.
  $value = kraftwagen_context_get_option('default-' . $option);
  if (!is_null($value)) {
    return (bool) $value;
  }

  return drush_confirm($msg);
}
