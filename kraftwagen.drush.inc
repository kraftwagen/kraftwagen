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

  $items['kraftwagen-create-project'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create a new project. This will setup the basic directory and settings stuff for you.'),
    'arguments' => array(
      'directory' => dt('The directory to create the project in. Defaults to the current directory'),
    ),
    'aliases' => array('kw-cp'),
    'options' => array(
      'force' => dt('Forces execution of this command, even if the specified source dir already exists.'),
    ),
  );

  $items['kraftwagen-build'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create a build from the source'),
    'aliases' => array('kw-b'),
  );

  $items['kraftwagen-make-update'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Update an existing makefile to the newest version of contributed modules. This will NOT do any major updates.'),
    'arguments' => array(
      'makefile.make' => dt('The makefile to update. As this command will actually change this file, it should be readable and writeable, and you should backup it, or have it under version control.')
    ),
    'required-arguments' => 1,
    'aliases' => array('kw-mu'),
  );

  $items['kraftwagen-dependencies'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'description' => dt('Check if all required modules are enabled, enable them if they are not, and disable them if the are enabled, but not required.'),
    'aliases' => array('kw-d'),
  );

  $items['kraftwagen-update'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'description' => dt('Update a Kraftwagen site'),
    'aliases' => array('kw-u'),
  );

  $items['kraftwagen-install'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
    'description' => dt('Install a Kraftwagen site'),
    'aliases' => array('kw-i'),
  );

  $items['kraftwagen-project-init'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Initialize a project. This will initialize the environment.'),
    'aliases' => array('kw-pi'),
  );

  $items['kraftwagen-build-init'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Initialize a build. This will link the environment to the build.'),
    'aliases' => array('kw-bi'),
  );

  $items['kraftwagen-environment-setup'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create an empty environment'),
    'aliases' => array('kw-es'),
  );
  $items['kraftwagen-environment-setup-settings'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Push settings into the environment.'),
    'aliases' => array('kw-ess'),
  );

  $items['kraftwagen-build-symlink'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create symlink from the environment into the drupal config directory.'),
    'aliases' => array('kw-bs'),
    'arguments' => array(
      'name' => dt('The name of the file to symlink.')
    ),
    'required-arguments' => 1,
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

function kraftwagen_commands_sequence_run($option) {
  $commands = kraftwagen_context_get_option($option);
  kraftwagen_commands_sequence_run_commands($commands);
}

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
      drush_invoke_process('@self', $command, $arguments);
    } else {
      drush_log(dt("Command !command not found", array('!command' => $command)), 'error');
    }
  }
}
