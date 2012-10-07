<?php

// Name of the Kraftwagen configuration file
define('KRAFTWAGEN_RC', 'kraftwagenrc.php'); 

// Prefix of all Kraftwagen configuration values
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
  if (file_exists(dirname(__FILE__) . '/' . KRAFTWAGEN_RC)) {
    kraftwagen_context_load_config_file(dirname(__FILE__) . '/' . KRAFTWAGEN_RC, KRAFTWAGEN_CONTEXT_DEFAULT);
  }
}

/**
 * Initialize the Kraftwagen root and load the kraftwagenrc.php file in it, if it 
 * exists.
 */
function kraftwagen_context_init_kraftwagen_root() {
  // try to find the root
  $root = kraftwagen_context_find_kraftwagen_root(getcwd());

  // if a root is found, initialize it
  if (!empty($root)) {
    kraftwagen_context_set_option('root-path', $root);
    if (file_exists($root . '/' . KRAFTWAGEN_RC)) {
      kraftwagen_context_load_config_file($root . '/' . KRAFTWAGEN_RC, KRAFTWAGEN_CONTEXT_ROOT);
    }
  } else {
    drush_set_error('kraftwagen', dt('Could not find a Kraftwagen root'));
  }
}

/**
 * Traverse upwards in the directory tree to find a Kraftwagen working directory.
 *
 * @param string $target Path to start the traversal
 * @return string The path of the Kraftwagen root, or FALSE if no root is found.
 */
function kraftwagen_context_find_kraftwagen_root($target) {
  // make sure we have a full and valid directory path to start with
  $dir = realpath($target);
  if (is_file($dir)) {
    $dir = dirname($dir);
  }
  if (!is_dir($dir)) {
    return FALSE;
  }

  // traverse upwards
  while ($dir) {

    // if the directory is a valid kraftwagen root, return it
    if (kraftwagen_context_check_directory($dir)) {
      return $dir;
    }

    // try to find the parent directory
    $prev_dir = $dir;
    $dir = dirname($dir);

    // dirname('/') == '/'
    if ($dir == $prev_dir) {
      return FALSE;
    }
  }

  return FALSE;
}

function _kraftwagen_context_check_directory_replace($matches) {
  $value = kraftwagen_context_get_option($matches[1]);
  return $value ? $value : '*ERROR*';
}

/**
 * Check if a directory is Kraftwagen root directory
 *
 * @param string $path The path to check for
 * @return bool TRUE if the path is Kraftwagen root directory, FALSE if not.
 */
function kraftwagen_context_check_directory($path) {
  $rc_file = FALSE;
  if (is_file($path . '/' . KRAFTWAGEN_RC)) {
    kraftwagen_context_load_config_file_temp($path . '/' . KRAFTWAGEN_RC);
    $rc_file = TRUE;
  }

  $return = TRUE;

  $root_checks = kraftwagen_context_get_option('root-checks');
  foreach ($root_checks as $check) {
    $func = 'kraftwagen_context_check_directory_' . $check['type'];
    if (function_exists($func)) {
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
    } else {
      $return = FALSE; break;
    }
  }

  if ($rc_file) {
    kraftwagen_context_restore_config();
  }

  return $return;
}

function kraftwagen_context_check_directory_require_directory($dir, $parameters) {
  return is_dir($dir . '/' . $parameters['dir']);
}
function kraftwagen_context_check_directory_require_file($dir, $parameters) {
  return is_file($dir . '/' . $parameters['file']);
}

/** 
 * Get the value of Kraftwagen option.
 *
 * A fairly simple but very convenient wrapper around drush_get_option().
 */
function kraftwagen_context_get_option($key, $default = NULL, $context = NULL) {
  return drush_get_option(KRAFTWAGEN_PREFIX . $key, $default, $context);
}

/**
 * Get the values of different kraftwagen options.
 */
function kraftwagen_context_get_options() {
  $args = func_get_args();

  $result = array();
  foreach ($args as $arg) {
    $result[] = kraftwagen_context_get_option($arg);
  }
  return $result;
}

/**
 * Set an Kraftwagen option in one of the option contexts.
 *
 * A fairly simple but very convenient wrapper around drush_set_option().
 */
function kraftwagen_context_set_option($key, $value, $context = KRAFTWAGEN_CONTEXT_RUNTIME) {
  return drush_set_option(KRAFTWAGEN_PREFIX . $key, $value, $context);
}

/**
 * Load a kraftwagen config file
 */
function kraftwagen_context_load_config_file($config, $context = KRAFTWAGEN_CONTEXT_RUNTIME) {
  if (file_exists($config)) {
    // fetch options from file
    $options = array();
    drush_log(dt('Loading kraftwagenrc "!config".', array('!config' => realpath($config))), 'kraftwagen');
    $ret = @include($config);
    if ($ret === FALSE) {
      drush_log(dt('Cannot open kraftwagenrc "!config", ignoring.', array('!config' => realpath($config))), 'warning');
      return FALSE;
    }

    // load options into context
    if (!empty($options)) {
      $drush_options = array();
      foreach ($options as $key => $value) {
        $drush_options[KRAFTWAGEN_PREFIX . $key] = $value;
      }
      $drush_options = array_merge(drush_get_context($context), $drush_options);
      drush_set_config_options($context, $drush_options, array());
    }
  }
}

/**
 * Get a key => value array of all available Kraftwagen config options
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
 * @param string $config The config file to load
 */
function kraftwagen_context_load_config_file_temp($config) {
  $data = _kraftwagen_context_config_store();

  // refuse to load the file, if another file is currently temporarily loaded
  if ($data !== FALSE) {
    drush_set_error('kraftwagen', dt('Could not load temporary config file !config, because another temporary config file is currently loaded.', array('!config' => $config)));
    exit;
  } else {
    // get the current config data and store it
    $data = _kraftwagen_context_get_config_options();
    _kraftwagen_context_config_store($data);

    // load the config file
    kraftwagen_context_load_config_file($config);
  }
}

function kraftwagen_context_confirm($option, $msg, $indent = 0) {
  // use default value for option
  $value = lenkrad_context_get_option('default-' . $option);
  if (!is_null($value)) {
    return (bool) $value;
  }

  return drush_confirm($msg, $indent);
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

      // remove the items that were not present before loading the file
      $keys = array_keys($cache);
      foreach ($keys as $key) {
        if (substr($key, 0, strlen(KRAFTWAGEN_PREFIX)) == KRAFTWAGEN_PREFIX) {
          if (!isset($data[$context][substr($key, strlen(KRAFTWAGEN_PREFIX))])) {
            unset($cache[$key]);
          }
        }
      }

      // set the values from the storage
      if (!empty($data[$context])) {
        foreach ($data[$context] as $key => $value) {
          $cache[KRAFTWAGEN_PREFIX . $key] = $value;
        }
      }
    }
  }

  // empty the storage
  _kraftwagen_context_config_store(FALSE);
}

/**
 * Get all Kraftwagen config options per context
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
 * Store a config array temporarily.
 */
function _kraftwagen_context_config_store($data = NULL) {
  static $store = FALSE;

  if (isset($data)) {
    $store = $data;
  }

  return $store;
}
