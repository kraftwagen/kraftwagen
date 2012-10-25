<?php

/**
 * @file
 * Kraftwagen provides drush command for a _everything in code_ and _install
 * profile_ based Drupal development workflow.
 */

/**
 * Implements hook_drush_command().
 */
function kraftwagen_drush_command() {
  $items = array();

  $items['kw-new-project'] = array( 
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create a new project. This will create an new source directory from a Git skeleton repository.'),
    'arguments' => array(
      'directory' => dt('The directory to create the project in. Defaults to the current directory'),
    ),
    'aliases' => array('kw-np'),
    'options' => array(
      'force' => dt('Forces execution of this command, even if the specified source dir already exists.'),
    ),
  );

  $items['kw-build'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create a build from the source'),
    'aliases' => array('kw-b'),
  );

  $items['kw-generate-makefile'] = array(
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Generate a new make file from the source'),
    'arguments' => array(
      'location' => dt('The location where to store the new makefile')
    ),
    'required-arguments' => 1,
  );

  $items['kw-activate-build'] = array(
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Link a symlink to the specified build'),
    'arguments' => array(
      'build' => dt('The location of the build'),
    ),
    'required-arguments' => 1,
  );

  $items['kw-update-makefile'] = array( 
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Update an existing makefile to the newest version of contributed modules. This will NOT do any major updates.'),
    'arguments' => array(
      'makefile.make' => dt('The makefile to update. As this command will actually change this file, it should be readable and writeable, and you should backup it, or have it under version control.')
    ),
    'required-arguments' => 1,
    'aliases' => array('kw-um'),
  );

  $items['kw-apply-module-dependencies'] = array( 
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'description' => dt('Check if all required modules are enabled, enable them if they are not, and disable them if the are enabled, but not required.'),
    'aliases' => array('kw-amd'),
  );

  $items['kw-update'] = array( 
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'description' => dt('Update a Kraftwagen project. Runs all commands necessary to bring the database in sync with code in the build.'),
    'aliases' => array('kw-u'),
  );

  $items['kw-init-database'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
    'description' => dt('Initialize (or re-initialize) the database for a Kraftwagen build.'),
    'aliases' => array('kw-id'),
  );

  $items['kw-setup'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Sets up the environment, settings files and uploaded files directory for a Kraftwagen project.'),
    'aliases' => array('kw-s'),
  );

  $items['kw-setup-env'] = array(
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Attempts to create an empty environment, defaulting to \'production\', for a Kraftwagen project.'),
  );

  $items['kw-setup-settings'] = array(
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Attempts to copy the settings files from the source into the correct location for a Kraftwagen project.'),
  );

  $items['kw-setup-upload-dir'] = array(
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Attempts to create the \'user uploaded files\' directory for a Kraftwagen project.'),
  );

  $items['kw-setup-symlinks'] = array(
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Creates or recreates symlinks for the settings files and the uploaded files directory into the specified Kraftwagen build.'),
    'arguments' => array(
      'build' => dt('The build to create the symlinks in. Defaults to the currently active build.'),
    ),
  );

  $items['kw-setup-symlink'] = array(
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Creates or recreates a symlink into the specified Kraftwagen build.'),
    'aliases' => array('kw-bs'),
    'arguments' => array(
      'build' => dt('The build to create the symlink in.'),
      'name' => dt('The name of the file or dir to symlink.')
    ),
    'required-arguments' => 2,
  );

  return $items;
}

/**
 * Implements hook_drush_init().
 */
function kraftwagen_drush_init() {
  // load the context handling and fill it with the default values in
  // kraftwagenrc.php.
  require_once dirname(__FILE__) . '/includes/kraftwagen.context.inc';
  kraftwagen_context_load_default_config();
}

/**
 * Run a sequence of drush commands that are defined in Kraftwagen context 
 * option.
 */
function kraftwagen_commands_sequence_run($option, $replacements = array()) {
  $commands = kraftwagen_context_get_option($option);
  $commands = kraftwagen_commands_sequence_replace($commands, $replacements);
  kraftwagen_commands_sequence_run_commands($commands);
}

/**
 * Run a sequence of drush commands.
 */
function kraftwagen_commands_sequence_run_commands($commands) {
  $available_commands = drush_get_context('DRUSH_COMMANDS');
  
  foreach ($commands as $command => $arguments) {
    if (is_numeric($command)) {
      if (is_array($arguments)) {
        kraftwagen_commands_sequence_run_commands($arguments);
        continue;
      } else {
        $command = $arguments;
        $arguments = array();
      }
    }
    if (isset($available_commands[$command])) {
      $result = drush_invoke_process('@self', $command, $arguments);
      if (!$result || !empty($result['error_status'])) {
        return drush_set_error(dt("Called command !command returned an error.", array('!command' => $command)));
      }
    } else {
      drush_log(dt("Command !command not found", array('!command' => $command)), 'error');
    }
  }
}

function kraftwagen_commands_sequence_replace($data, $replacements) {
  if (is_array($data)) {
    $result = array();
    foreach ($data as $key => $value) {
      $key = kraftwagen_commands_sequence_replace($key, $replacements);
      $value = kraftwagen_commands_sequence_replace($value, $replacements);
      $result[$key] = $value;
    }
    return $result;
  } elseif (is_string($data)) {
    return str_replace(array_keys($replacements), array_values($replacements), $data);
  } else {
    return $data;
  }
}
