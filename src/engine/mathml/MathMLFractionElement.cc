// Copyright (C) 2000-2003, Luca Padovani <luca.padovani@cs.unibo.it>.
//
// This file is part of GtkMathView, a Gtk widget for MathML.
// 
// GtkMathView is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// GtkMathView is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GtkMathView; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// For details, see the GtkMathView World-Wide-Web page,
// http://helm.cs.unibo.it/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>

#include <cassert>

#include "ChildList.hh"
#include "MathFormattingContext.hh"
#include "MathGraphicDevice.hh"
#include "Globals.hh"
#include "MathMLDummyElement.hh"
#include "MathMLFormattingEngineFactory.hh"
#include "MathMLFractionElement.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLView.hh"
#include "ValueConversion.hh"
#include "MathMLAttributeSignatures.hh"

MathMLFractionElement::MathMLFractionElement(const SmartPtr<class MathMLView>& view)
  : MathMLContainerElement(view)
{
}

MathMLFractionElement::~MathMLFractionElement()
{
  SetNumerator(0);
  SetDenominator(0);
}

void
MathMLFractionElement::SetNumerator(const SmartPtr<MathMLElement>& elem)
{
  if (elem != numerator)
    {
      numerator = elem;
      if (numerator) elem->setParent(this);
      setDirtyLayout();
    }
}

void
MathMLFractionElement::SetDenominator(const SmartPtr<MathMLElement>& elem)
{
  if (elem != denominator)
    {
      denominator = elem;
      if (denominator) elem->setParent(this);
      setDirtyLayout();
    }
}

#if 0
void
MathMLFractionElement::Replace(const SmartPtr<MathMLElement>& oldElem, const SmartPtr<MathMLElement>& newElem)
{
  assert(oldElem);
  if (oldElem == numerator) SetNumerator(newElem);
  else if (oldElem == denominator) SetDenominator(newElem);
  else assert(false);
}
#endif

void
MathMLFractionElement::construct()
{
  if (dirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      if (getDOMElement())
	{
	  assert(IsA() == T_MFRAC);
	  ChildList children(getDOMElement(), MATHML_NS_URI, "*");
	  unsigned n = children.get_length();

	  if (n > 0)
	    SetNumerator(getFormattingNode(children.item(0)));
	  else if (!numerator || !is_a<MathMLDummyElement>(numerator))
	    SetNumerator(getFactory()->createDummyElement(getView()));

	  if (n > 1)
	    SetDenominator(getFormattingNode(children.item(1)));
	  else if (!denominator || !is_a<MathMLDummyElement>(denominator))
	    SetDenominator(getFactory()->createDummyElement(getView()));
	}
#endif

      if (numerator) numerator->construct();
      if (denominator) denominator->construct();

      resetDirtyStructure();
    }
}

void
MathMLFractionElement::refine(AbstractRefinementContext& context)
{
  if (dirtyAttribute() || dirtyAttributeP())
    {
      REFINE_ATTRIBUTE(context, MathML, Fraction, linethickness);
      REFINE_ATTRIBUTE(context, MathML, Fraction, numalign);
      REFINE_ATTRIBUTE(context, MathML, Fraction, denomalign);
      REFINE_ATTRIBUTE(context, MathML, Fraction, bevelled);
      if (GetNumerator()) GetNumerator()->refine(context);
      if (GetDenominator()) GetDenominator()->refine(context);
      MathMLContainerElement::refine(context);
    }
}

#if 0
void
MathMLFractionElement::Setup(RenderingEnvironment& env)
{
  if (dirtyAttribute() || dirtyAttributeP())
    {
      color = env.GetColor();
      background = env.GetBackgroundColor();

#ifdef TEXISH_MATHML
      defaultRuleThickness = env.GetRuleThickness();
#else
      scaled defaultRuleThickness = env.GetRuleThickness();
#endif // TEXISH_MATHML
      
      if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(Fraction, linethickness))
	{
	  if (IsTokenId(value))
	    {
	      switch (ToTokenId(value))
		{
		case T_THIN:
		  lineThickness = defaultRuleThickness / 2;
		  break;
		case T_MEDIUM:
		  lineThickness = defaultRuleThickness;
		  break;
		case T_THICK:
		  lineThickness = defaultRuleThickness * 2;
		  break;
		default:
		  assert(IMPOSSIBLE);
		  break;
		}
	    }
	  else if (IsNumber(value))
	    {
	      lineThickness = defaultRuleThickness * ToNumber(value);
	    }
	  else
	    {
	      Length unitValue = ToLength(value);
	      if (unitValue.type == Length::PERCENTAGE_UNIT)
		lineThickness = defaultRuleThickness * unitValue.value / 100;
	      else
		lineThickness = env.ToScaledPoints(unitValue);
	    }
	  
	  lineThickness = std::max(scaled(0), lineThickness);
	}
      
      if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(Fraction, numalign))
	numAlign = ToTokenId(value);

      if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(Fraction, denomalign))
	denomAlign = ToTokenId(value);

      if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(Fraction, bevelled))
	bevelled = ToBoolean(value);

      color = env.GetColor();

      axis = env.GetAxis();

      displayStyle = env.GetDisplayStyle();

#ifdef TEXISH_MATHML
      if (displayStyle) {
	numMinShift = env.GetFontAttributes().size.ToScaledPoints() * 0.676508;
	denomMinShift = env.GetFontAttributes().size.ToScaledPoints()) * 0.685951;
      } else {
	numMinShift = env.GetFontAttributes().size.ToScaledPoints() * (lineThickness > 0 ? 0.393732 : 0.443731);
	denomMinShift = env.GetFontAttributes().size.ToScaledPoints() * 0.344841;
      }
#else
      minShift = env.GetScaledPointsPerEx();
#endif // TEXISH_MATHML

      env.Push();
      if (!displayStyle) env.AddScriptLevel(1);
      else env.SetDisplayStyle(false);

      if (numerator) numerator->Setup(env);
      if (denominator) denominator->Setup(env);

      env.Drop();
      resetDirtyAttribute();
    }
}

void
MathMLFractionElement::DoLayout(const class FormattingContext& ctxt)
{
  if (dirtyLayout(ctxt))
    {
      assert(numerator && denominator);
      numerator->DoLayout(ctxt);
      denominator->DoLayout(ctxt);

      const BoundingBox& numBox   = numerator->GetBoundingBox();
      const BoundingBox& denomBox = denominator->GetBoundingBox();

      if (bevelled) {
	scaled barVert = std::max(numBox.verticalExtent(), denomBox.verticalExtent());
	scaled barHoriz = barVert / 2;

	box.set(barHoriz + 2 * lineThickness, 0, 0);
	box.append(numBox);
	box.append(denomBox);
      } else {
#ifdef TEXISH_MATHML
	scaled u = numMinShift;
	scaled v = denomMinShift;
#else
	scaled u = minShift;
	scaled v = minShift;
#endif // TEXISH_MATHML

	if (lineThickness == scaled(0)) {
#ifdef TEXISH_MATHML
	  scaled psi = (displayStyle ? 7 : 3) * defaultRuleThickness;
#else
	  scaled psi = displayStyle ? 3 * lineThickness : lineThickness;
#endif // TEXISH_MATHML
	  scaled phi = (u - numBox.depth) - (denomBox.height - v);

	  if (psi < phi) {
	    u += (phi - psi) / 2;
	    v += (phi - psi) / 2;
	  }
	} else {
	  scaled phi = displayStyle ? 3 * lineThickness : lineThickness;

	  scaled diff = phi - ((u - numBox.depth) - (axis + lineThickness / 2));
	  if (diff > scaled(0)) u += diff;

	  diff = phi - ((axis - lineThickness / 2) - (denomBox.height - v));
	  if (diff > scaled(0)) v += diff;
	}

	numShift   = u;
	denomShift = v;
    
	box.set(std::max(numBox.width, denomBox.width),
		numShift + numBox.height,
		denomShift + denomBox.depth);
      }

      DoEmbellishmentLayout(this, box);

      resetDirtyLayout(ctxt);
    }
}
#endif

AreaRef
MathMLFractionElement::format(MathFormattingContext& ctxt)
{
  if (dirtyLayout())
    {
      Length thickness;
      if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(MathML, Fraction, linethickness))
	{
	  if (IsTokenId(value))
	    {
	      switch (ToTokenId(value))
		{
		case T_THIN:
		  thickness.set(0.5, Length::PURE_UNIT);
		  break;
		case T_MEDIUM:
		  thickness.set(1, Length::PURE_UNIT);
		  break;
		case T_THICK:
		  thickness.set(2, Length::PURE_UNIT);
		  break;
		default:
		  assert(false);
		  break;
		}
	    }
	  else if (IsNumber(value))
	    thickness.set(ToNumber(value), Length::PURE_UNIT);
	  else
	    thickness = ToLength(value);
	}
      else
	assert(false);

      TokenId numAlign = ToTokenId(GET_ATTRIBUTE_VALUE(MathML, Fraction, numalign));
      TokenId denomAlign = ToTokenId(GET_ATTRIBUTE_VALUE(MathML, Fraction, denomalign));
      bool bevelled = ToBoolean(GET_ATTRIBUTE_VALUE(MathML, Fraction, bevelled));

      AreaRef res;

      ctxt.push(this);
      if (ctxt.getDisplayStyle()) ctxt.setDisplayStyle(false);
      else ctxt.addScriptLevel(1);
      AreaRef num = GetNumerator()->format(ctxt);
      AreaRef denom = GetDenominator()->format(ctxt);

      if (bevelled)
	res = ctxt.getDevice()->bevelledFraction(ctxt, num, denom, thickness);
      else
	{
	  switch (numAlign)
	    {
	    case T_LEFT: break; // nothing to do
	    case T_CENTER: num = ctxt.getDevice()->getFactory()->center(num); break;
	    case T_RIGHT: num = ctxt.getDevice()->getFactory()->right(num); break;
	    default: assert(false);
	    }

	  switch (denomAlign)
	    {
	    case T_LEFT: break; // nothing to do
	    case T_CENTER: denom = ctxt.getDevice()->getFactory()->center(denom); break;
	    case T_RIGHT: denom = ctxt.getDevice()->getFactory()->right(denom); break;
	    default: assert(false);
	    }
	
	  res = ctxt.getDevice()->fraction(ctxt, num, denom, thickness);
	}

      res = formatEmbellishment(this, ctxt, res);

      setArea(ctxt.getDevice()->wrapper(ctxt, res));

      ctxt.pop();
      resetDirtyLayout();
    }

  return getArea();
}

#if 0
void
MathMLFractionElement::SetPosition(const scaled& x0, const scaled& y0)
{
  scaled x = x0;
  scaled y = y0;

  position.x = x;
  position.y = y;

  SetEmbellishmentPosition(this, x, y);

  assert(numerator && denominator);

  const BoundingBox& box      = GetBoundingBox();
  const BoundingBox& numBox   = numerator->GetBoundingBox();
  const BoundingBox& denomBox = denominator->GetBoundingBox();

  if (bevelled) {
    scaled barVert = std::max(numBox.verticalExtent(), denomBox.verticalExtent());
    scaled barHoriz = barVert / 2;

    numerator->SetPosition(x, y);
    denominator->SetPosition(x + numBox.width + barHoriz + 2 * lineThickness, y);
  } else {
    scaled numXOffset = 0;
    switch (numAlign) {
    case T_CENTER:
      numXOffset = (box.width - numBox.width) / 2;
      break;
    case T_RIGHT:
      numXOffset = box.width - numBox.width;
      break;
    default:
      numXOffset = 0;
      break;
    }

    scaled denomXOffset = 0;
    switch (denomAlign) {
    case T_CENTER:
      denomXOffset = (box.width - denomBox.width) / 2;
      break;
    case T_RIGHT:
      denomXOffset = box.width - denomBox.width;
      break;
    default:
      denomXOffset = 0;
      break;
    }

    numerator->SetPosition(x + numXOffset, y - numShift);
    denominator->SetPosition(x + denomXOffset, y + denomShift);
  }
}

void
MathMLFractionElement::Render(const DrawingArea& area)
{
  if (Exposed(area))
    {
      if (fGC[Selected()] == NULL)
	{
	  GraphicsContextValues values;
	  values.foreground = Selected() ? area.GetSelectionForeground() : color;
	  values.lineWidth = lineThickness;
	  fGC[Selected()] = area.GetGC(values, GC_MASK_FOREGROUND | GC_MASK_LINE_WIDTH);
	}

      RenderBackground(area);

      assert(numerator && denominator);
      numerator->Render(area);
      denominator->Render(area);

      if (lineThickness != scaled(0))
	{
	  if (bevelled)
	    {
	      const BoundingBox& numBox   = numerator->GetBoundingBox();
	      const BoundingBox& denomBox = denominator->GetBoundingBox();

	      scaled barVert = std::max(numBox.verticalExtent(), denomBox.verticalExtent());
	      scaled barHoriz = barVert / 2;

	      area.DrawLine(fGC[Selected()],
			    GetX() + numBox.width + lineThickness,
			    GetY() + std::max(numBox.depth, denomBox.depth),
			    GetX() + numBox.width + lineThickness + barHoriz,
			    GetY() - std::max(numBox.height, denomBox.height));
	    }
	  else
	    {
	      area.DrawLine(fGC[Selected()], GetX(), GetY() - axis,
			    GetX() + box.width, GetY() - axis);
	    }
	}

      ResetDirty();
    }
}
#endif

void
MathMLFractionElement::setFlagDown(Flags f)
{
  MathMLElement::setFlagDown(f);
  if (numerator) numerator->setFlagDown(f);
  if (denominator) denominator->setFlagDown(f);
}

void
MathMLFractionElement::resetFlagDown(Flags f)
{
  MathMLElement::resetFlagDown(f);
  if (numerator) numerator->resetFlagDown(f);
  if (denominator) denominator->resetFlagDown(f);
}

#if 0
scaled
MathMLFractionElement::GetLeftEdge() const
{
  return 0;
}

scaled
MathMLFractionElement::GetRightEdge() const
{
  return box.width;
}

void
MathMLFractionElement::ReleaseGCs()
{
  MathMLElement::ReleaseGCs();
  if (numerator) numerator->ReleaseGCs();
  if (denominator) denominator->ReleaseGCs();
}

SmartPtr<MathMLElement>
MathMLFractionElement::Inside(const scaled& x, const scaled& y)
{
  if (!IsInside(x, y)) return 0;

  SmartPtr<MathMLElement> inside = 0;
  if (numerator && (inside = numerator->Inside(x, y))) return inside;
  if (denominator && (inside = denominator->Inside(x, y))) return inside;

  return this;
}
#endif

SmartPtr<MathMLOperatorElement>
MathMLFractionElement::GetCoreOperator()
{
  if (numerator) return numerator->GetCoreOperator();
  else return 0;
}