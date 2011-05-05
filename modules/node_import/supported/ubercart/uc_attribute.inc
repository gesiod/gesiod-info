<?php
// $Id

/**
 * @file
 * Support file for Ubercart Attributes
 */

/**
 * Implementation of hook_node_import_fields().
 * 
 * Add the Attributes for the given type.
 */
function uc_attribute_node_import_fields($type) 
{
	// If we're a Node & a Product type...
	if (($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type)) 
	{		
		foreach(_uc_attribute_node_import_get_types_attributes($node_type) as $aid => $label)
		{
			$fields['attribute:'.$aid] = array(
	    		'title'			=> t($label),
	    		'group' 		=> t('Product Attributes'),
				'is_mappable'	=> TRUE,
				'weight'		=> 1,
			);
		}
		
		// New sub SKU for the attribute combination.
		$fields['sub_sku'] = array(
    		'title'			=> t('Sub-SKU'),
    		'group' 		=> t('Product Attributes'),
			'is_mappable'	=> TRUE,
			'weight'		=> 2,
			'tips'			=> array('The SKU used for each combination of attributes. Product SKU will be used for grouping them together.')
		);
		return $fields;
	}
}

/**
 * Get a list of Attributes for the given $node_type
 * @param	$node_type	String, type of the node.
 * @return 				Array, keys are Attr. IDs and values are Attr. label.
 */
function _uc_attribute_node_import_get_types_attributes($node_type)
{
	$out = array();
	
	// Get attributes list for the $type
	$query = db_query("
		SELECT	aid, label
		FROM	{uc_class_attributes}
		WHERE	pcid = '%s'", 
	$node_type);

	while($attr = db_fetch_object($query))
	{
		$out[$attr->aid] = $attr->label;
	}	
	return $out;
}

/**
 * Implementation of hook_node_import_types_alter()
 * 
 * Need to change the uc_products create function to one of ours as we
 * don't have one row = one node anymore and if the SKU exists we need
 * to skip the node creation and just do attribute creation instead.
 */
function uc_attribute_node_import_types_alter(&$types_list)
{
	foreach($types_list as $type => $info)
	{
		if(($node_type = node_import_type_is_node($type)) !== FALSE && uc_product_is_product($node_type))
		{
			$types_list[$type]['create'] = 'uc_attribute_product_create_wrapper';
		}
	}

}

/**
 * Wrapper function around the Products create function to test if the product already exists.
 * 
 * @param $type			String, Node Imports internal Node 'type'
 * @param $values		Array, values from the CSV row
 * @param $preview		Boolean, if TRUE Node Import is asking for a preview of the row.
 * @return 				mixed, if $preview = TRUE then HTML string, otherwise object ID of new object (i.e. vid, uid, etc).
 */
function uc_attribute_product_create_wrapper($type, $values, $preview)
{
	// See if it exists already
	if (! $nid = uc_attribute_get_product_by_sku($values['model']) )
	{
		// By adding a Node ID it _should_ cause an UPDATE instead of an INSERT.
		// A new node revisoin will be created? Not what we want really :-/
//		$values['nid'] = $nid; 	// don't work :(

		// Return to regular schedule...
		$output = node_import_create_node($type, $values, $preview);

	}
	else
	{
		$values['nid'] = $nid;
		
		// TODO: then do an update instead of insert... somehow
	}

	if ($preview)
	{
		$output = node_import_create_node($type, $values, $preview);
	}

	return $output;
}

/**
 * Look up Product by SKU. Check if it exists.
 * 
 * @param $sku	The SKU (model) of the Product we're interested in.
 * @return Integer, the NID of the Product with the matching SKU (model), FALSE otherwise.
 */
function uc_attribute_get_product_by_sku($sku)
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

function get_nid_list()
{
	return new IDList('node_import_uc_attribute_temp.txt');
}

/**
 * Implementation of hook_node_import_postprocess().
 * 
 * We're using this hook as we need a NID and this should only be available post 'create'
 */
function uc_attribute_node_import_postprocess($type, $values, $options, $preview)
{		
	// If we're a Node & a Product type...
	if (!$preview 
		&& ($node_type = node_import_type_is_node($type)) !== FALSE // ...is a node...
		&& uc_product_is_product($node_type) 						// ...is a product type...
		&& uc_attribute_has($node_type)								// ...has attributes.
	)							
	{
		// List of NIDs we've already processed
		$NIDs = get_nid_list();

		// Make sure we have a NID, incase this was a product that we skipped creating coz it already existed.
		if (!isset($values['nid']))
			$nid = uc_attribute_get_product_by_sku($values['model']);
		else
			$nid = $values['nid'];

		// If its a new NID to us then clean out its Options settings...	
		if(!$NIDs->exists($nid))
		{
			// Delete the default attribute options which is all on by default...
			db_query("DELETE FROM {uc_product_options} WHERE nid = %d", $nid);
			
			// ...and the adjustments table values
			db_query("DELETE FROM {uc_product_adjustments} WHERE nid = %d", $nid);
			
			// Add the nid to the list of NIDs we've seen.
			$NIDs->add($nid);
		}
		
		// Create the Attribute option for this combo...
		
		// Get attributes for this Product Class, note that this whole .inc is designed for
		// Product Classes only, not Products with any old attribute attached. + Some static cache'n
		$class_attrs = uc_class_get_attributes($node_type);

		$attrs_present = array();
		
		foreach($class_attrs as $aid => $attr)
		{
			// If there is a value for this attribute...
			if (isset($values['attribute:'.$aid]) AND !empty($values['attribute:'.$aid]))
			{
				$new_option = strtolower(trim($values['attribute:'.$aid]));
				
				// ... and its a valid option value...
				
				$valid_option = FALSE;
				foreach($attr->options as $oid => $option)
					if(strtolower(trim($option->name)) == $new_option)
					{
						$valid_option = $oid;
						break;
					}
					
				// if we don't know about this attribute option...
				
				if(!$valid_option)
				{	// ... Create it in the DB, which means...

					// ... create record in {uc_attribute_options} table...
					db_query('INSERT INTO {uc_attribute_options} (aid, name) VALUES (%d, \'%s\')', $aid, $new_option);
					$new_oid = db_result(db_query("SELECT LAST_INSERT_ID()"));
					// TODO: should do this but can't be bothered testing it.
					// $new_oid = db_last_insert_id('uc_attribute_options', 'oid')	
					
					// ... and then in the {uc_class_attributes_options} table.
					db_query('INSERT IGNORE INTO {uc_class_attribute_options} (pcid, oid) VALUES (\'%s\', %d)', $node_type, $new_oid);
										
					// Don't forget to get the new OID so we can update the {uc_product_options} table :-/
					$valid_option = $new_oid;
				}

				// Finally add the Option to the new Product Instance
				db_query('INSERT IGNORE INTO {uc_product_options} (nid, oid) VALUES (%d, %d)', $nid, $valid_option);
				
/* might just do this on the front end... :-/
				// Make sure a default is selected
				if(!db_result(db_query('SELECT default_option FROM {uc_product_attributes} WHERE nid = %d AND aid = %d', $nid, $aid)))
				{
					db_query('UPDATE {uc_product_attributes} SET default_option = 1 WHERE nid = %d AND aid = %d LIMIT 1', $nid, $aid);
				}
*/
				$attrs_present[$aid] = $valid_option;
			}
		}
				
		// If all Attributes got an Option then make a record in the Adjustments table.
//		if(!count(array_diff( array_keys($class_attrs), array_keys($attrs_present) )))		
		{
			db_query("INSERT INTO {uc_product_adjustments} (nid, combination, model) VALUES (%d, '%s', '%s')", $nid, serialize($attrs_present), $values['sub_sku']);
		}
	}
}

/**
 * Check if a Product class has Attributes.
 * @param 	$class	String representing the product class ID (pcid). Not $type as used by node_import
 * @return	boolean	TRUE if the class has attributes, FALSE otherwise.
 */
function uc_attribute_has($class)
{
	return db_result(db_query("SELECT aid FROM {uc_class_attributes} WHERE pcid = '%s'", $class));
}

/**
 * Class to create and monitor a list of IDs. in a text file for cross session tracking
 * Used here to track NIDs we've already processed to avoid erasing attributes we've already created.
 */
class IDList
{
	/**
	 * Full path to the file
	 */
	private $_file_path;
	
	/**
	 * Internal list of IDs for quick search
	 */
	private $_id_list;
	
	public function __construct($filename = 'id_list.txt')
	{
		$this->_file_path = file_directory_path() . '/' . $filename;
		
		if(file_exists($this->_file_path))
		{
			$this->_id_list = explode(',', file_get_contents($this->_file_path));
		}
		else
		{
			$this->_id_list = array();
		}
	}
	
	public function add($id)
	{
		$this->_id_list[] = $id;
	}
	
	public function exists($id)
	{
		return in_array($id, $this->_id_list);
	}
	
	/**
	 * Empty before end of execution to delete file.
	 */
	public function clear()
	{
		$this->_id_list = array();
	}

	public function __destruct() 
	{
		if(count($this->_id_list))
		{
			if (!file_put_contents($this->_file_path, implode(',', $this->_id_list), LOCK_EX)) die ('Error, cannot create temp file');
		}	
		else
			if(file_exists($this->_file_path))
			{
				unlink($this->_file_path);
			}
	}
}
