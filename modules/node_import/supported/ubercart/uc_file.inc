<?php
// $Id: uc_product.inc,v 1.1.2.3 2009/10/09 15:23:21 robrechtj Exp $

/**
 * Implementation of hook_node_import_fields().
 */
function uc_file_node_import_fields($type) {
  $fields = array();

  if (($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type)) {

    if (!is_dir(variable_get('uc_file_base_dir', NULL))) {
      /* User must initialize settings for uc_file module before node import will work for uc_file */
      drupal_set_message(t('A file directory needs to be configured in <a href="!url">product feature settings</a> before a file can be selected.', array('!url' => url('admin/store/settings/products/edit/features'))), 'error');
      return $fields;
    }

    $fields['uc_file'] = array(
        'title' => t('File download'),
        'group' => t('Ubercart File Download'),
        'input_format' => 'uc_file',
        'to_directory' => variable_get('uc_file_base_dir', NULL),
        'tips' => array(t('Path to file for "pay for download"')),
        );  
    $fields['uc_file_description'] = array(
        'title' => t('Download Description'),
        'group' => t('Ubercart File Download'),
        'tips' => array(t('Description for file download')),
        );
    $fields['uc_file_download_limit'] = array(
        'title' => t('Download Limit'),
        'group' => t('Ubercart File Download'),
        'tips' => array(t('Override the amount of times a customer can download this file after the product has been purchased.')),
        );
    $fields['uc_file_address_limit'] = array(
        'title' => t('Download IP Address Limit'),
        'group' => t('Ubercart File Download'),
        'tips' => array(t('Override the amount of locations (IP addresses) a customer can download this file from after the product has been purchased.')),
        );
    $fields['uc_file_time_quantity'] = array(
        'title' => t('Download Time Quantity'),
        'group' => t('Ubercart File Download'),
        'tips' => array(t('Override the amount of time a customer can download this file after the product has been purchased.')),
        );
    $fields['uc_file_time_granularity'] = array(
        'title' => t('Download Time Granularity'),
        'group' => t('Ubercart File Download'),
        'tips' => array(t('Override the amount of time a customer can download this file after the product has been purchased.')),
        'allowed_values' => array(
          'never' => t('never'), 'day' => t('day(s)'), 'week' => t('week(s)'), 'month' => t('month(s)'), 'year' => t('year(s)')
          ),
        );

    return $fields;
  }
}

function uc_file_node_import_postprocess($type, $values, $options, $preview) {
  //dpm(array('uc_file_node_import_postprocess' => $type, 'values' => $values, 'options' => $options, 'preview' => $preview));
  if (($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type)) {
    if(!$preview) {
      // Make sure we have a NID, incase this was a product that we skipped creating coz it already existed.
      if (!isset($values['nid'])) {
        // when you enable the uc_attribute.inc stuff, if there are duplicate
        // sku's you don't get an NID back. This is a problem, as we need an NID.
        $nid = uc_file_node_import_get_product_by_sku($values['model']);
      }
      else {
        $nid = $values['nid'];
      }

      if(!$nid) {
        drupal_set_message(t("Don't have nid, can not attach uc_file", array("@dst" => $dst_dir)));
        return;
      }

      uc_file_node_import_add_feature($nid, $values['uc_file'], $values);
    }
  }
}

function uc_file_node_import_add_feature($nid, $path, $values = array()) {
    $dst_dir = variable_get('uc_file_base_dir', NULL);
    $filename = basename($path);

    $src_filepath = node_import_directory() . '/' . $path;
    $dst_filepath = $dst_dir . '/' . $filename;

    if(!(is_file($src_filepath) || is_file($dst_filepath))) {
      // file not found. Can't import
      drupal_set_message(t("Unable to file source (@src)", array("@src" => $src_filepath)));
      return;
    }
    if(!is_dir($dst_dir)) {
      // won't be able to import file.
      drupal_set_message(t("Destination folder does not exist (@dst)", array("@dst" => $dst_dir)));
      return;
    }

    // If the file doesn't already exist in download folder, copy it there
    if(!is_file($dst_filepath)) {
      if(!copy($src_filepath, $dst_filepath)) {
        drupal_set_message(t('an error occurred while copying the file to %dir', array('%dir' => $dst_dir)));
        return;
      }
    }

    // Update the file list
    uc_file_refresh();
    $uc_file = uc_file_get_by_name($filename);

    // Clean up these variables
    if(!empty($values['uc_file_time_granularity'])) {
      if($values['uc_file_time_granularity'] == 'never') {
        $values['uc_file_time_quantity'] = 0;
      }
      else {
        if($values['uc_file_time_quantity'] <= 0) { 
          // If time granularity is set and not never, but quantity is less than 0
          // fail quietly and set to default values.
          // TODO: Warn user instead of error
          $values['uc_file_time_quantity'] = UC_FILE_LIMIT_SENTINEL;
          $values['uc_file_time_granularity'] = UC_FILE_LIMIT_SENTINEL;
        }
      }
    }

    // If sub-sku is set, use that, else use default model
    $values['sku'] = !empty($values['sub_sku']) ? $values['sub_sku'] : $values['model'];

    $file_product = array(
        'fid'         => $uc_file->fid,
        'filename'    => $uc_file->filename,
        'pfid'        => '', #$form_state['values']['pfid'],
        'model'       => !empty($values['sku']) ? $values['sku'] : '',
        'description' => !empty($values['uc_file_description']) ? $values['uc_file_description'] : '',
        'shippable'   => isset($values['shippable']) ? $values['shippable'] : 0,

        // Local limitations... set them if there's an override.
        'download_limit'    => !empty($values['uc_file_download_limit'])   ? $values['uc_file_download_limit']   : UC_FILE_LIMIT_SENTINEL,
        'address_limit'     => !empty($values['uc_file_address_limit'])    ? $values['uc_file_address_limit']    : UC_FILE_LIMIT_SENTINEL,
        // Extra case is to allow time_quantity to be 0 when granuliarty is never
        'time_quantity'     => (!empty($values['uc_file_time_quantity']) || ($values['uc_file_time_granularity'] == 'never' && $values['uc_file_time_quantity'] == 0)) ? $values['uc_file_time_quantity']    : UC_FILE_LIMIT_SENTINEL,
        'time_granularity'  => !empty($values['uc_file_time_granularity']) ? $values['uc_file_time_granularity'] : UC_FILE_LIMIT_SENTINEL,
        );

    // Build product feature descriptions.
    $description = t('<strong>SKU:</strong> !sku<br />', array('!sku' => empty($file_product['model']) ? 'Any' : $file_product['model']));
    if (is_dir(variable_get('uc_file_base_dir', NULL) ."/". $file_product['filename'])) {
      $description .= t('<strong>Directory:</strong> !dir<br/>', array('!dir' => $file_product['filename']));
    }
    else {
      $description .= t('<strong>File:</strong> !file<br/>', array('!file' => basename($file_product['filename'])));;
    }
    $description .= $file_product['shippable'] ? t('<strong>Shippable:</strong> Yes') : t('<strong>Shippable:</strong> No');

    $data = array(
        'pfid' => 0,
        'nid' => $nid,
        'fid' => 'file',
        'description' => $description,
        );

    // There's a bug in this function that needs to get patched: http://drupal.org/node/809318
    uc_product_feature_save($data);

  // Insert or update uc_file_product table
  if (empty($file_product['pfid'])) {
    $file_product['pfid'] = db_last_insert_id('uc_product_features', 'pfid');
  }

  $key = NULL;
  if ($fpid = _uc_file_get_fpid($file_product['pfid'])) {
    $key = 'fpid';
    $file_product['fpid'] = $fpid;
  }

  drupal_write_record('uc_file_products', $file_product, $key);
}

/**
 * Look up Product by SKU. Check if it exists.
 * DUPLICATE of uc_attribute_get_product_by_sku()
 * 
 * @param $sku	The SKU (model) of the Product we're interested in.
 * @return Integer, the NID of the Product with the matching SKU (model), FALSE otherwise.
 */
function uc_file_node_import_get_product_by_sku($sku)
{
	static $nid_list = array();

	if(!in_array($sku, $nid_list))
	{
		$nid = db_result(db_query("SELECT nid FROM {uc_products} WHERE model = '%s'", $sku));
		
		if($nid)
			$nid_list[$sku] = $nid;
		
		return $nid;
	}
	else
	{
		return $nid_list[$sku];
	}
}

/**
 * Implementation of hook_node_import_defaults().
 */
/* Left in here incase someone wants to enable this in the future. Personally I think falling
 * back on the defaults is good enough.

function uc_file_node_import_defaults($type, $defaults, $fields, $map) {
  $form = array();

  // If they're not mapped to anything, we just default to UC_FILE_LIMIT_SENTINEL
  return $form;

  if (($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type)) {
    $form['uc_file_download_limit'] = array(
        '#type' => 'textfield',
        '#title' => t('Downloads'),
        '#description' => t("The number of times a file can be downloaded. Leave empty to set no limit."),
        '#default_value' => variable_get('uc_file_download_limit_number', NULL),
        '#maxlength' => 4,
        '#size' => 4,
        '#weight' => 1,
        );
    $form['uc_file_address_limit'] = array(
        '#type' => 'textfield',
        '#title' => t('IP addresses'),
        '#description' => t("The number of unique IPs that a file can be downloaded from. Leave empty to set no limit."),
        '#default_value' => variable_get('uc_file_download_limit_addresses', NULL),
        '#maxlength' => 4,
        '#size' => 4,
        '#weight' => 2,
        );
    $form['uc_file_time_quantity'] = array(
        '#type' => 'textfield',
        '#title' => t('Time'),
        '#default_value' => variable_get('uc_file_download_limit_duration_qty', NULL),
        '#size' => 4,
        '#maxlength' => 4,
        '#prefix' => '<div class="duration">',
        '#suffix' => '</div>',
        '#weight' => 3,
        );
    $form['uc_file_time_granularity'] = array(
        '#type' => 'select',
        '#options' => array(
          'never' => t('never'),
          'day' => t('day(s)'),
          'week' => t('week(s)'),
          'month' => t('month(s)'),
          'year' => t('year(s)')
          ),
        '#default_value' => variable_get('uc_file_download_limit_duration_granularity', 'never'),
        '#description' => t('How long after a product has been purchased until its file download expires.'),
        '#prefix' => '<div class="duration">',
        '#suffix' => '</div>',
        '#weight' => 4,
        );
  }
  return $form;
}
*/

