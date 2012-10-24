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
  return kraftwagen_commands_sequence_run('setup-commands');
}