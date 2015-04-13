<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-update`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-update`.
 */
function drush_kw_update_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND() for `drush kw-update`.
 */
function drush_kraftwagen_kw_update() {
  require_once dirname(__FILE__) . '/includes/kraftwagen.environment.inc';
  $environment = kraftwagen_context_environment();

  kraftwagen_commands_sequence_run('update-commands', array(
    '*environment*' => $environment,
  ), FALSE);
}
