
sgDagNode * Maya::exportWorld( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & transform )
{
	transform = new sgTransformMatrix( *graph );
	transform->name = "$world";
	transform->update();
	return parent->insert( *transform );
}

