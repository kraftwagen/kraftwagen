<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-setup`.
 */

/**
 * Implements drush_COMMAND for `drush kw-setup`.
 */
function drush_kraftwagen_kw_setup() {
  //Give a chance to determine environment using drush argument
  $default_environment = drush_get_option(array('env'));

  if(!$default_environment) {
    $default_environment = kraftwagen_context_get_option('environment-default');
  }

  $replacement['*environment*'] = $default_environment;

  return kraftwagen_commands_sequence_run('setup-commands', $replacement);
}
