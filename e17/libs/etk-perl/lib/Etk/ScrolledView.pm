package Etk::ScrolledView;
use strict;
use vars qw(@ISA);
require Etk::Widget;
@ISA = ("Etk::Widget");
sub new
{
    my $class = shift;
    my $self = $class->SUPER::new();
    $self->{WIDGET} = Etk::etk_scrolled_view_new();
    bless($self, $class);
    return $self;
}

sub AddWithViewport
{
    my $self = shift;
    my $widget = shift;
    Etk::etk_scrolled_view_add_with_viewport($self->{WIDGET}, $widget->{WIDGET});
}

sub HScrollBarGet
{
    my $self = shift;
    return Etk::etk_scrolled_view_hscrollbar_get($self->{WIDGET});
}

sub VScrollBarGet
{
    my $self = shift;
    return Etk::etk_scrolled_view_vscrollbar_get($self->{WIDGET});
}

sub PolicyGet
{
}

sub PolicySet
{
}

1;
