// Fill out your copyright notice in the Description page of Project Settings.


#include "System/CMPReplicationGraphSettings.h"

#include "System/CMPReplicationGraph.h"

UCMPReplicationGraphSettings::UCMPReplicationGraphSettings()
{
	CategoryName = TEXT("Game");
	DefaultReplicationGraphClass = UCMPReplicationGraph::StaticClass();
}
