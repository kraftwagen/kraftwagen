<?php

/**
 * @file
 * This file contains the functions that are required to execute 
 * `drush kraftwagen-update`.
 */

/**
 * Implements drush_COMMAND() for `drush kraftwagen-update`.
 */
function drush_kraftwagen_update() {
  kraftwagen_commands_sequence_run('update-commands');
}
