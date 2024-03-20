// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(Blueprintable)
enum EInterpType
{
	Linear UMETA(DisplayName = "Linear"),
	EaseIn UMETA(DisplayName = "EaseIn"),
	EaseOut UMETA(DisplayName = "EaseOut"),
	EaseInOut UMETA(DisplayName = "EaseInOut"),
	CircularIn UMETA(DisplayName = "CircularIn"),
	CircularOut UMETA(DisplayName = "CircularOut"),
	CircularInOut UMETA(DisplayName = "CircularInOut"),
	ExpoIn UMETA(DisplayName = "Expoin"),
	ExpoOut UMETA(DisplayName = "ExpoOut"),
	ExpoInOut UMETA(DisplayName = "ExpoInOut"),
	SinIn UMETA(DisplayName = "SinIn"),
	SinOut UMETA(DisplayName = "SinOut"),
	SinInOut UMETA(DisplayName = "SinInOut"),
};

UENUM(Blueprintable)
enum EInterpToTargetType
{
	Constant UMETA(DisplayName = "Constant"),
	InterpTo UMETA(DisplayName = "InterpTo"),
	InterpStep UMETA(DisplayName = "InterpStep"),
};

template<typename T>
T Interp(EInterpType InInterpType, T InMinRange, T InMaxRange, float Alpha, float InterpControl = 2)
{
	//switch on the interpolation type
	switch (InInterpType)
	{
		//linear interpolation
		case Linear:
		{
			//return the linear interpolation
			return FMath::Lerp(InMinRange, InMaxRange, Alpha);
		}
		//EaseIn interpolation
		case EaseIn:
		{
			//return the EaseIn interpolation
			return FMath::InterpEaseIn(InMinRange, InMaxRange, Alpha, InterpControl);
		}
		//EaseOut interpolation
		case EaseOut:
		{
			//return the EaseOut interpolation
			return FMath::InterpEaseOut(InMinRange, InMaxRange, Alpha, InterpControl);
		}
		//EaseInOut interpolation
		case EaseInOut:
		{
			//return the EaseInOut interpolation
			return FMath::InterpEaseInOut(InMinRange, InMaxRange, Alpha, InterpControl);
		}
		//CircularIn interpolation
		case CircularIn:
		{
			//return the CircularIn interpolation
			return FMath::InterpCircularIn(InMinRange, InMaxRange, Alpha);
		}
		//CircularOut interpolation
		case CircularOut:
		{
			//return the CircularOut interpolation
			return FMath::InterpCircularOut(InMinRange, InMaxRange, Alpha);
		}
		//CircularInOut interpolation
		case CircularInOut:
		{
			//return the CircularInOut interpolation
			return FMath::InterpCircularInOut(InMinRange, InMaxRange, Alpha);
		}
		//ExpoIn interpolation
		case ExpoIn:
		{
			//return the ExpoIn interpolation
			return FMath::InterpExpoIn(InMinRange, InMaxRange, Alpha);
		}
		//ExpoOut interpolation
		case ExpoOut:
		{
			//return the ExpoOut interpolation
			return FMath::InterpExpoOut(InMinRange, InMaxRange, Alpha);
		}
		//ExpoInOut interpolation
		case ExpoInOut:
		{
			//return the ExpoInOut interpolation
			return FMath::InterpExpoInOut(InMinRange, InMaxRange, Alpha);
		}
		//SinIn interpolation
		case SinIn:
		{
			//return the SinIn interpolation
			return FMath::InterpSinIn(InMinRange, InMaxRange, Alpha);
		}
		//SinOut interpolation
		case SinOut:
		{
			//return the SinOut interpolation
			return FMath::InterpSinOut(InMinRange, InMaxRange, Alpha);
		}
		//SinInOut interpolation
		case SinInOut:
		{
			//return the SinInOut interpolation
			return FMath::InterpSinInOut(InMinRange, InMaxRange, Alpha);
		}
		//default case
		default:
		{
			//return the linear interpolation
			return FMath::Lerp(InMinRange, InMaxRange, Alpha);
		}
	}
}

template<typename T>
T InterpToTarget(EInterpToTargetType InInterpType, T Current, T Target, float DeltaTime, float InterpSpeed)
{
	switch (InInterpType)
	{
		//constant interpolation
		case Constant:
		{
			//return the constant interpolation
			return FMath::FInterpConstantTo(Current, Target, DeltaTime, InterpSpeed);
		}
		//InterpTo interpolation
		case InterpTo:
		{
			//return the acceleration interpolation
			return FMath::FInterpTo(Current, Target, DeltaTime, InterpSpeed);
		}
		//InterpStep interpolation
		case InterpStep:
		{
			//return the acceleration interpolation
			return FMath::FInterpTo(Current, Target, DeltaTime, InterpSpeed);
		}
		//default
		default:
		{
			//return the acceleration interpolation
			return FMath::FInterpTo(Current, Target, DeltaTime, InterpSpeed);
		}
	}
}


