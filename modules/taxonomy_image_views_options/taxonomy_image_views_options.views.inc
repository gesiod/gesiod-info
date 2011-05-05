<?php
// $Id: taxonomy_image_views_options.views.inc,v 1.1 2011/02/09 21:01:48 scyther Exp $

/**
 * @file
 * Views integration for Taxonomy Image Views Options module.
 */

/**
 * Implementation of hook_views_data().
 */
function taxonomy_image_views_options_views_data_alter(&$data) {
  $data['term_image']['tid']['field']['handler'] = 'views_handler_field_taxonomy_image_views_options';
}
