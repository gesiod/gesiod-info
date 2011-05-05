<?php
// $Id: uc_product.inc,v 1.1.2.2 2009/02/25 21:16:10 robrechtj Exp $

/**
 * @file
 * Support for the Ubercart Stock module.
 */

/**
 * Implementation of hook_node_import_types().
 
function uc_stock_node_import_types()
{
	$types = array();
	
	$types['uc_stock'] = array(
		'title' => t('Stock'),
		'can_create' => TRUE,
		'create' => 'node_import_create_stock_levels',
	);

  return $types;
}
*/

/**
 * Implementation of hook_node_import_fields().
 * 
 * Add the Stock fields to the Product type forms.
 */
function uc_stock_node_import_fields($type) 
{
	// If we're a Node & a Product type...
	if (($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type)) 
	{
		$fields['active'] = array(
    		'title'			=> t('Active'),
    		'input_format'	=> 'boolean',
    		'group' 		=> t('Product Stock'),
    		'weight' 		=> 15,
			'is_mappable'	=>  TRUE
    	);
    	$fields['stock'] = array(
    		'title'			=> t('Stock'),
    		'group' 		=> t('Product Stock'),
    		'weight' 		=> 16,
    		'is_mappable'	=>  TRUE
    	);
    	$fields['threshold'] = array(
    		'title'			=> t('Threshold'),
    		'group' 		=> t('Product Stock'),
    		'weight' 		=> 17,
    		'is_mappable'	=>  TRUE
    	);
	}
	return $fields;
}

/**
 * Implementation of hook_node_import_defaults().
 * 
 * Let the user choose some defaults.
 */
function uc_stock_node_import_defaults($type, $defaults, $fields, $map) 
{
	$form = array();
	
	// If we're a Node & a Product type...
	if (($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type)) 
	{
       $form['active'] = array(
          '#title' 				=> 'Active',
          '#type' 				=> 'radios',
          '#options' 			=> array(0 => t('No'), 1 => t('Yes')),
          '#default_value' 		=> 1,
        );
		$form['stock'] = array(
			'#title' 			=> 'Stock',
			'#type' 			=> 'textfield',
			'#default_value' 	=> 0,
			'#size'				=> 10,
		);
		$form['threshold'] = array(
			'#title' 			=> 'Threshold',
			'#type' 			=> 'textfield',
			'#default_value' 	=> 1,
			'#size'				=> 10,
		);
	}
	return $form;
}

/**
 * Implementation of hook_node_import_postprocess().
 * 
 * Create the corresponding stock levels entry for the current product row.
 */
function uc_stock_node_import_postprocess($type, $values, $options, $preview)
{	
	// If we're a Node & a Product type...
	if ( ($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type) )
	{
		foreach(array('active', 'stock', 'threshold') as $key)		
			$data[$key] = $values[$key];
		
		$data['active'] = ($data[$key]) ? 1 : 0;	// just make sure.
			
		// NID is a PK in the {uc_product_stock} table.
		if(!isset($values['nid']))
			$data['nid'] = uc_stock_node_import_get_product_by_sku($values['model']);

		if(module_exists('uc_attribute') && uc_attribute_has($node_type))
			$data['sku'] = $values['sub_sku'];
		
		if ($data['sku'] && $data['nid'] && !$preview) 
		{
			// We have to do this coz db_query(UPDATE) always returs 1 ? 
			if($test = db_result(db_query('SELECT nid FROM {uc_product_stock} WHERE sku = \'%s\'', $data['sku'])))
			{
				db_query(
					'UPDATE {uc_product_stock} SET active = %d, stock = %d, threshold = %d WHERE sku = \'%s\' AND nid = %d',
					$data['active'], $data['stock'], $data['threshold'], $data['sku'], $data['nid']
				);
				drupal_set_message('Stock UPDATED: '.print_r($data, true));				
			}	
			else
			{
				db_query(
					'INSERT INTO {uc_product_stock} (active, stock, threshold, sku, nid) VALUES (%d, %d, %d, \'%s\', %d)',
					$data['active'], $data['stock'], $data['threshold'], $data['sku'], $data['nid']
				);
				drupal_set_message('Stock INSERTed: '.print_r($data, true));
			}
//			drupal_set_message('Stock TEST nid: '.print_r($test, true));
		}
		else
			drupal_set_message('Stock data: '.print_r($data, true));
	}
}

/**
 * Look up Product by SKU. Check if it exists.
 * DUPLICATE of uc_attribute_get_product_by_sku()
 * 
 * @param $sku	The SKU (model) of the Product we're interested in.
 * @return Integer, the NID of the Product with the matching SKU (model), FALSE otherwise.
 */
function uc_stock_node_import_get_product_by_sku($sku)
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
