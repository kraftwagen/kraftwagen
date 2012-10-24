<?php

/**
 * @file
 * This file contains the functions that are required to execute 
 * `drush kw-update`.
 */

/**
 * Implements drush_COMMAND() for `drush kw-update`.
 */
function drush_kraftwagen_kw_update() {
  kraftwagen_commands_sequence_run('update-commands');
}
