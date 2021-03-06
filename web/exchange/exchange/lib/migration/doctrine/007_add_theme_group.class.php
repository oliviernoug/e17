<?php
/**
 * This class has been auto-generated by the Doctrine ORM Framework
 */
class AddThemeGroup extends Doctrine_Migration
{
	public function up()
	{
		$this->createTable('theme_group', array (
  'id' => 
  array (
    'primary' => true,
    'autoincrement' => true,
    'type' => 'integer',
    'length' => 4,
  ),
  'name' => 
  array (
    'type' => 'string',
    'length' => 255,
  ),
  'title' => 
  array (
    'type' => 'string',
    'length' => 255,
  ),
  'known' => 
  array (
    'type' => 'boolean',
    'length' => 25,
  ),
  'created_at' => 
  array (
    'type' => 'timestamp',
    'length' => 25,
  ),
  'updated_at' => 
  array (
    'type' => 'timestamp',
    'length' => 25,
  ),
), array (
  'indexes' => 
  array (
  ),
  'primary' => 
  array (
    0 => 'id',
  ),
));
	}

	public function down()
	{
		$this->dropTable('theme_group');
	}
}